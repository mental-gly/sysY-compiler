#include "AST/Stmt.h"

static llvm::LLVMContext Context;
static llvm::IRBuilder<> Builder(Context);
static std::unique_ptr<llvm::Module> Module;
// keep tracks values defined in the current scope
static std::map<std::string, llvm::Value *> NamedValues;



llvm::Value *BinaryOperator::CodeGen() {
    Operands[0] = SubExprs[0]->CodeGen();
    Operands[1] = SubExprs[1]->CodeGen();
    if (Operands[0] == nullptr || Operands[1] == nullptr)
        return nullptr;
    // neglect Twine name
    assert(Opcode != Uknown && "Unknown Binary Operation Opcode!");
    switch (Opcode) {
        case Add    :   return Builder.CreateAdd(Operands[0], Operands[1]);
        case FAdd   :   return Builder.CreateFAdd(Operands[0], Operands[1]);   
        case Sub    :   return Builder.CreateSub(Operands[0], Operands[1]);
        case FSub   :   return Builder.CreateFSub(Operands[0], Operands[1]);
        case Mul    :   return Builder.CreateMul(Operands[0], Operands[1]);
        case FMul   :   return Builder.CreateFMul(Operands[0], Operands[1]);
        case UDiv   :   return Builder.CreateUDiv(Operands[0], Operands[1]);
        case SDiv   :   return Builder.CreateSDiv(Operands[0], Operands[1]);
        case FDiv   :   return Builder.CreateFDiv(Operands[0], Operands[1]);
        case URem   :   return Builder.CreateURem(Operands[0], Operands[1]);
        case SRem   :   return Builder.CreateSRem(Operands[0], Operands[1]);
        case FRem   :   return Builder.CreateFRem(Operands[0], Operands[1]);
    } 
}




IntegerLiteral::IntegerLiteral(TypeInfo *T, uint64_t val, bool isSigned=false) 
    : ExprStmt(T, RValue),
        Value(T->ByteSize * 8, val, isSigned) 
{
    assert(8 % T->ByteSize == 0 && "IntegerLiteral supports 1, 2, 4, 8 btyes integer only!");
    Type = llvm::IntegerType::get(Context, T->ByteSize * 8);
}

IntegerLiteral::IntegerLiteral(TypeInfo *T, llvm::StringRef valStr, uint8_t radix)
    : ExprStmt(T, RValue),
        Value(T->ByteSize * 8, valStr, radix)
{
    assert(8 % T->ByteSize == 0 && "IntegerLiteral supports 1, 2, 4, 8 btyes integer only!");
    Type = llvm::IntegerType::get(Context, T->ByteSize * 8);
}

llvm::Value *IntegerLiteral::CodeGen() {
    return llvm::ConstantInt::get(Context, Value);
}










FloatingLiteral::FloatingLiteral(TypeInfo *T, double val) 
    : ExprStmt(T, RValue),
        Value(val)
{
    assert(T->ByteSize * 8 == 64 && "Not a double Type T");
    Type = llvm::Type::getDoubleTy(Context);
}

FloatingLiteral::FloatingLiteral(TypeInfo *T, float val)
    : ExprStmt(T, RValue),
        Value(val)
{   
    assert(T->ByteSize * 8 == 64 && "Not a double Type T");
    Type = llvm::Type::getFloatTy(Context);
}


llvm::Value *FloatingLiteral::CodeGen() {
    return llvm::ConstantFP::get(Context, Value);
}