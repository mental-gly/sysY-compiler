#include "AST/Decl.h"
#include "AST/TypeInfo.h"

//===-- CompileUnit --===//
CompileUnitDecl::CompileUnitDecl(Decl *decls, const std::string &FileName)
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
    else LOG(WARNING) << FileName << " has nothing to compile";
}


//===-- FunctionDecl --===//

FunctionDecl::FunctionDecl(TypeInfo *return_type, const std::string &name, ParamDecl *params)
    : Decl(name)
{
    ReturnType = return_type;
    auto param = params;
    CHECK(param) << "Empty parameter list";
    while (params->Next != nullptr) {
        Params.push_back(param);
        // We guarantee it's a safe static_cast.
        param = static_cast<ParamDecl *>(param->Next);
    }
}

bool FunctionDecl::hasDefinition(CompileUnitDecl *U) const {
    auto F = U->Module->getFunction(Name);
    if (F == nullptr || F->empty())
        return false;
    return true;
}


llvm::Function *FunctionDecl::CodeGen(CompileUnitDecl *U) {
    // Collecting function return type and param type info.
    llvm::SmallVector<llvm::Type *, 10> ParamT;
    for (const auto param : Params) {
        ParamT.push_back(param->getType()->Type);
    }
    llvm::Type *RetT = ReturnType->Type;
    // Currently, we do not support var args.
    llvm::FunctionType *FunT;
    if (Params.size() == 0) {
        FunT = llvm::FunctionType::get(RetT, ParamT, false);
    } else {
        FunT = llvm::FunctionType::get(RetT, false);
    }

    // If the function has a body, we should create definition in LLVM IR.
    // This function act in 3 cases:
    //  1. if this function do not exist, add a prototype and return this function.
    //  2. if the existing function has the correct prototype, return the existing one.
    //  3. if the existing function has the wrong prototype, the function is a constantexpr cast to right prototype.
    llvm::FunctionCallee FWrapper = U->Module->getOrInsertFunction(Name, FunT);
    if (auto F = dyn_cast<llvm::Function>(FWrapper.getCallee())) {
        // If the FunctionDecl has a Stmt body,
        // generate the code, the F would be automatically
        // transformed from \p declare into a \p define
        if (hasBody()) {
            U->Symbol.CreateScope();
            auto *Entry = llvm::BasicBlock::Create(*U->Context, "entry", F);
            U->Builder->SetInsertPoint(Entry);
            // store return val.
            llvm::AllocaInst *RetValAlloca = nullptr;
            if (!F->getReturnType()->isVoidTy()) {
                RetValAlloca = U->Builder->CreateAlloca(F->getReturnType(),
                                         0, llvm::Twine(F->getName(), "ret"));
            }

            // Store Args.
            for (auto &Arg : F->args()) {
                llvm::AllocaInst *Alloca = U->Builder->CreateAlloca(Arg.getType(),
                                         0, Arg.getName());
                // add to symbol table.
                U->Symbol[Arg.getName()] = Alloca;
                U->Builder->CreateStore(&Arg, Alloca);
            }
            // Generate definition.
            Body->CodeGen(U);
            // deal with final ret,
            // if there is no ret, we add a final ret.
            if (!llvm::isa<llvm::ReturnInst>(F->getBasicBlockList().back().back())) {
                auto *FinalRet = llvm::BasicBlock::Create(*U->Context, "ret", F);
                U->Builder->CreateBr(FinalRet);
                U->Builder->SetInsertPoint(FinalRet);
                // need load the return val.
                llvm::Value *RetVal = nullptr;
                if (!F->getReturnType()->isVoidTy()) {
                    RetVal = U->Builder->CreateLoad(F->getReturnType(), RetValAlloca);
                }
                U->Builder->CreateRet(RetVal);
            }
            U->Builder->ClearInsertionPoint();
            U->Symbol.LeaveScope();
        }
    } else {
        LOG(FATAL) << "Conflicting type for " << "'" << Name << "'";
    }
}