#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

int main() {
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("test", *context);
    auto builder = std::make_unique<IRBuilder<>>(*context);

    Type *IntT = Type::getInt32Ty(*context);
    PointerType::get(IntT,0)->print(outs(), true);
    Constant *Idx = ConstantInt::get(IntT, 0);
    Constant *Idx2 = ConstantInt::get(IntT, 0);
    ArrayType *ArrayIntT = ArrayType::get(IntT, 10);
    Value *A = module->getOrInsertGlobal("A", ArrayIntT);
    A->print(outs(), true);
    outs() << "\n";
    Value *Test = ConstantArray::get(ArrayIntT, {Idx});
    Test->print(outs(), true);
    outs() << "\n";
    Value *GEP = builder->CreateGEP(ArrayIntT, A, {Idx, Idx2});
    GEP->print(outs(), true);
    outs() << "\n";

    Value *GEP2 = builder->CreateGEP(IntT, A, Idx );
    GEP2->print(outs(), true);
    outs() << "\n";
    Value *GEP3 = builder->CreateGEP(Test,  Idx);
    GEP3->print(outs(), true);
    outs() << "\n";
}