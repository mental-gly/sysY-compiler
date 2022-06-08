#include "AST/Decl.h"
#include "AST/TypeInfo.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

//===-- CompileUnit --===//
CompileUnitDecl::CompileUnitDecl(const std::string &FileName, Decl *decls)
    :Decl(FileName)
{
    // initialize Context, Builder and Module.
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>(Name, *Context);
    Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
    // Set declaration list in this compile unit.
    if (decls != nullptr) {
        auto decl = decls;
        while (decl != nullptr) {
            Decls.push_back(decl);
            decl = decl->Next;
        }
    }
}

void CompileUnitDecl::CreateSubDecls(Decl *DeList) {
    // Set declaration list in this compile unit.
    if (DeList != nullptr) {
        for (auto decl = DeList; decl != nullptr; decl = decl->Next)
            Decls.push_back(decl);
    }
}

void CompileUnitDecl::CodeGen() {
    if (Decls.empty()) {
        LOG(WARNING) << Name << " has nothing to compile\n";
    }
    else {
        for (auto Decl: Decls) {
            static_cast<FunctionDecl *>(Decl)->CodeGen(this);
        }
    }
}

void CompileUnitDecl::print() const {
    Module->print(llvm::outs(), nullptr);
}

//===-- VarDecl --===//
llvm::Value *VarDecl::CodeGen(CompileUnitDecl *U) {
    auto builder = U->getBuilder();
    auto context = U->getContext();
    auto F = builder->GetInsertBlock()->getParent();
    auto EntryBB = &F->getEntryBlock();
    auto EntryTerminator = EntryBB->getTerminator();
    auto CurBB = builder->GetInsertBlock();
    // make local variable alloca.
    if (EntryTerminator != nullptr) {
        builder->SetInsertPoint(EntryTerminator);
    }
    else {
        builder->SetInsertPoint(EntryBB);
    }
    auto LocalVarAlloca =  builder->CreateAlloca(getType()->Type, 0, Name);
    if (init_expr != nullptr) {
        auto InitExpr = init_expr->CodeGen(U);
        builder->CreateStore(InitExpr, LocalVarAlloca);
    }
}

//===-- FunctionDecl --===//

FunctionDecl::FunctionDecl(TypeInfo *return_type, const std::string &name, ParamDecl *params)
    : Decl(name)
{
    ReturnType = return_type;
    for (auto param = params; param != nullptr; param = static_cast<ParamDecl *>(param->Next)) {
        Params.push_back(param);
    }
    Body = nullptr;
}

bool FunctionDecl::hasDefinition(CompileUnitDecl *U) const {
    auto module = U->getModule();
    auto F = module->getFunction(Name);
    if (F == nullptr || F->empty())
        return false;
    return true;
}

/// \brief This helper function replace return instruction with stores;
/// This is due to the problem a early return would implicitly create a new BB
/// in LLVM IR.
static void doRARWS(llvm::IRBuilder<> *builder, llvm::Function *F,
                    bool isVoid, llvm::Value *RetValAddr, llvm::BasicBlock *RetBB)
{
    // Get the final ret value alloc inst
    auto RetName = llvm::Twine(F->getName(), "ret");
    llvm::SmallVector<char, 10> Name;
    auto RetT = F->getReturnType();
    llvm::SmallVector<llvm::Instruction *, 10> WorkList;
    // iterating over instructions in the Function.
    for (auto I = llvm::inst_begin(F), E = llvm::inst_end(F); I != E; ++I) {
        // We only deals with ReturnInst
        if (llvm::Instruction::isTerminator(I->getOpcode())) {
            if (llvm::isa<llvm::ReturnInst>(&*I)) {
                auto RetInst = llvm::dyn_cast<llvm::ReturnInst>(&*I);
                auto BB = RetInst->getParent();
                if (!isVoid) {
                    builder->SetInsertPoint(RetInst);
                    // Replace return with a store to ValueAddr.
                    auto Store = builder->CreateStore(RetInst->getOperand(0), RetValAddr);
                    RetInst->replaceAllUsesWith(Store);
                }
                WorkList.push_back(RetInst);
                builder->SetInsertPoint(BB);
                builder->CreateBr(RetBB);
            }
        }
    }
    for (auto DeadInst : WorkList) {
        DeadInst->eraseFromParent();
    }
}

llvm::Function *FunctionDecl::CodeGen(CompileUnitDecl *U) {
    auto module = U->getModule();
    auto builder = U->getBuilder();
    auto context = U->getContext();
    // Collecting function return type and param type info.
    llvm::SmallVector<llvm::Type *, 10> ParamT;
    for (const auto param : Params) {
        ParamT.push_back(param->getType()->Type);
    }
    llvm::Type *RetT = ReturnType->Type;
    // Currently, we do not support var args.
    llvm::FunctionType *FunT;
    if (Params.size() != 0) {
        FunT = llvm::FunctionType::get(RetT, ParamT, false);
    } else {
        FunT = llvm::FunctionType::get(RetT, false);
    }

    // If the function has a body, we should create definition in LLVM IR.
    // This function act in 3 cases:
    //  1. if this function do not exist, add a prototype and return this function.
    //  2. if the existing function has the correct prototype, return the existing one.
    //  3. if the existing function has the wrong prototype, the function is a constantexpr cast to right prototype.
    llvm::FunctionCallee FWrapper = module->getOrInsertFunction(Name, FunT);
    if (auto F = llvm::dyn_cast<llvm::Function>(FWrapper.getCallee())) {
        // If the FunctionDecl has a Stmt body,
        // generate the code, the F would be automatically
        // transformed from \p declare into a \p define
        if (hasBody()) {
            U->Symbol.CreateScope();
            auto *Entry = llvm::BasicBlock::Create(*context, "entry", F);
            builder->SetInsertPoint(Entry);
            // store return val.
            llvm::AllocaInst *RetValAlloca = nullptr;
            auto FinalRetBB = llvm::BasicBlock::Create(*context, "ret");
            if (!F->getReturnType()->isVoidTy()) {
                RetValAlloca = builder->CreateAlloca(F->getReturnType(),
                                         0, llvm::Twine(F->getName(), "ret"));
            }

            // Store Args.
            for (auto &Arg : F->args()) {
                LOG(INFO) << "In function " << F->getName().str() << " Param: " << Arg.getName().str() << "\n";
                llvm::AllocaInst *Alloca = builder->CreateAlloca(Arg.getType(),
                                         0, Params[Arg.getArgNo()]->getName());
                // add to symbol table.
                U->Symbol[Arg.getName()] = Alloca;
                builder->CreateStore(&Arg, Alloca);
            }
            // Generate definition.
            Body->CodeGen(U);
            // do Replace All Return With Store.
            doRARWS(builder, F, F->getReturnType()->isVoidTy(), RetValAlloca, FinalRetBB);
            // deal with final ret,
            // if there is no ret, we add a final ret.
            F->getBasicBlockList().push_back(FinalRetBB);
            auto CurrentFinalBB = builder->GetInsertBlock();
            if (!llvm::Instruction::isTerminator(CurrentFinalBB->back().getOpcode()))
                builder->CreateBr(FinalRetBB);
            builder->SetInsertPoint(FinalRetBB);
            // need load the return val.
            llvm::Value *RetVal = nullptr;
            if (!F->getReturnType()->isVoidTy()) {
                RetVal = builder->CreateLoad(F->getReturnType(), RetValAlloca);
            }
            builder->CreateRet(RetVal);
            builder->ClearInsertionPoint();
            U->Symbol.LeaveScope();

            llvm::verifyFunction(*F, &llvm::errs());
            return F;
        }
    }
    LOG(FATAL) << "Conflicting type for " << "'" << Name << "'";
    return nullptr;
}