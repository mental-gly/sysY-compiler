#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "logging.h"
#include <memory>
using namespace llvm;

int main() {
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("test", *context);
    auto builder = std::make_unique<IRBuilder<>>(*context);

    Type *RetT = Type::getInt32Ty(*context);
    Type *WrongRetT = Type::getVoidTy(*context);

    FunctionType *FunT = FunctionType::get(RetT, false);
    FunctionType *WrongFunT = FunctionType::get(WrongRetT, false);
    LOG(INFO) << "Dump created function type:\n";
    FunT->print(outs(), true);
    WrongFunT->print(outs(), true);
    FunctionCallee F = module->getOrInsertFunction("F", FunT);
    FunctionCallee WrongF = module->getOrInsertFunction("F", WrongFunT);
    CHECK_EQ(dyn_cast<Function>(WrongF.getCallee()), nullptr);
    LOG(INFO) << "Dump inserted function type:\n";
    F.getFunctionType()->print(outs(), true);
    WrongF.getFunctionType()->print(outs(), true);
    LOG(INFO) << "Create BB:\n";
    Function *Func = dyn_cast<Function>(F.getCallee());
    BasicBlock *BB = BasicBlock::Create(*context, "entry", Func);
    builder->SetInsertPoint(BB);
    Value *Ptr = builder->CreateAlloca(Type::getInt32Ty(*context), 0, "alloca");
    auto *FinalRet = llvm::BasicBlock::Create(*context, "ret", Func);
    builder->CreateBr(FinalRet);
    builder->SetInsertPoint(FinalRet);
    Value *Load = builder->CreateLoad(Ptr, "load");
    Value *RetVal = ConstantInt::getSigned(RetT, 0);
    builder->CreateRet(RetVal);
    builder->CreateRet(nullptr);
    auto SymbolTable = Func->getValueSymbolTable();
    LOG(INFO) << "Dump module";
    module->print(outs(), nullptr);
}