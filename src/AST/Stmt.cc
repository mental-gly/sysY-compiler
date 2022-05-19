#include "AST/Stmt.h"

//===-- DeclStmt --===//
DeclStmt::DeclStmt(VarDecl *DeList) {
    auto Var = DeList;
    while (Var != nullptr) {
        Decls.push_back(Var);
        Var = static_cast<VarDecl *>(Var->Next);
    }
}

void DeclStmt::setType(TypeInfo *type) {
    for (auto Var : Decls) {
        Var->setType(type);
    }
}


//===-- CompoundStmt --===//
CompoundStmt::CompoundStmt(Stmt *StList) {
    auto SubStmt = StList;
    while (SubStmt != nullptr) {
        Stmts.push_back(SubStmt);
        SubStmt = SubStmt->Next;
    }
}

void CompoundStmt::CreateSubStmt(Stmt *SubStmt) {
    Stmts.push_back(SubStmt);
}

llvm::Value *CompoundStmt::CodeGen(CompileUnitDecl *U) {
    // Compoundstmt inside is a new scope,
    // Which is managed by CompoundStmt.
    U->Symbol.CreateScope();
    llvm::ReturnInst *RetInstTrack;
    for (auto SubStmt : Stmts) {
        SubStmt->CodeGen(U);
        // optimization : ignore statements after ret.
        if (!ReturnStmt::classof(SubStmt)) break;
    }
    U->Symbol.LeaveScope();
    return nullptr;
}

//===-- IfStmt --===//



//===-- CallStmt --===//
CallStmt::CallStmt(const std::string &Func, ExprStmt *ArgList)
    : Callee(Func)
{
    auto Arg = ArgList;
    while (Arg != nullptr) {
        Args.push_back(Arg);
        Arg = static_cast<ExprStmt *>(Arg->Next);
    }
}

TypeInfo *CallStmt::getType(CompileUnitDecl *U) {
    if (ExprType != nullptr)
        return ExprType;
}

llvm::Value *CallStmt::CodeGen(CompileUnitDecl *U) {
    llvm::Function *F = U->Module->getFunction(Callee);
    CHECK(F) << "Call unknown function " << Callee;
    llvm::SmallVector<llvm::Value *, 10> ArgsV;
    for (ExprStmt *&Expr : Args) {
      ArgsV.push_back(Expr->CodeGen(U));
    }
    return U->Builder->CreateCall(F, ArgsV, llvm::Twine("Call", Callee));
}

//===-- DeclRefStmt --===//

DeclRefStmt::DeclRefStmt(const std::string &symbol)
    : ExprStmt(ExprStmt::LValue), Symbol(symbol) { }

TypeInfo *DeclRefStmt::getType(CompileUnitDecl *U) {
    // If we have searched symbol of the Ref.
    if (ExprType != nullptr)
        return ExprType;
}

/// \brief Load variable into virtual register.
llvm::Value *DeclRefStmt::CodeGen(CompileUnitDecl *U) {
    // The referenced symbol must be a alloca addr or global value addr,
    llvm::Value *AddrOrGlob = U->Symbol.lookup(Symbol);
    CHECK_NE(AddrOrGlob, nullptr) << "Undeclared variable " << Symbol;
    CHECK((llvm::dyn_cast<llvm::GlobalValue *>(AddrOrGlob) != nullptr)
          || (llvm::dyn_cast<llvm::AllocaInst *>(AddrOrGlob) != nullptr)) << "Not a local or global variable.";
    llvm::LoadInst *Load = U->Builder->CreateLoad(AddrOrGlob);
    return Load;
}

//===-- ArraySubscriptStmt --===//

ArraySubscriptStmt::ArraySubscriptStmt(ExprStmt *base, ExprStmt *idx)
    : ExprStmt(ExprStmt::LValue)
{
  Base = base;
  Idx = idx;
}

TypeInfo *ArraySubscriptStmt::getType(CompileUnitDecl *U) {
    if (ExprType != nullptr)
        return ExprType;
}

llvm::Value *ArraySubscriptStmt::CodeGen(CompileUnitDecl *U) {
    auto BaseVal = Base->CodeGen(U);
    auto IdxVal = Base->CodeGen(U);
    CHECK(BaseVal->getType()->isArrayTy() || BaseVal->getType()->isPointerTy())
        << "'" << reinterpret_cast<DeclRefStmt *>(BaseVal)->getSymbolName().str() << "'"
        << " should be an array or a pointer type";
    return U->Builder->CreateGEP(BaseVal, IdxVal);
}

//===-- ReturnStmt --===//
llvm::Value *ReturnStmt::CodeGen(CompileUnitDecl *U) {
    // If the function has non-void ret value,
    // we alloca a space for ret value;
    // We will create an extra BasicBlock
    // which load the RetVal and do the final actual return.
    //
    // It's an alternative to add a final ret BB, and let
    // any internal return br to this final BB.
    llvm::Value *RetV = RetExpr->CodeGen(U);
    U->Builder->CreateRet(RetV);
}




//===-- BinaryOperator --===//

TypeInfo *BinaryOperator::getType() {
    if (ExprType != nullptr)
        return ExprType;
}

llvm::Value *BinaryOperator::CodeGen(CompileUnitDecl *U) {
    Operands[0] = SubExprs[0]->CodeGen(U);
    Operands[1] = SubExprs[1]->CodeGen(U);
    if (Operands[0] == nullptr || Operands[1] == nullptr)
        return nullptr;
    // considering implicit casting
    if (Operands[0]->getType() != Operands[1]->getType()) {

    }
    // neglect Twine name
    CHECK_NE(Opcode, Unknown) << "Unknown Binary Operation Opcode!";
    switch (Opcode) {
        case Add    :   return U->Builder->CreateAdd(Operands[0], Operands[1]);
        case Sub    :   return U->Builder->CreateSub(Operands[0], Operands[1]);
        case Mul    :   return U->Builder->CreateMul(Operands[0], Operands[1]);
    }
}


//===-- IntegerLiteral --===//

IntegerLiteral::IntegerLiteral(TypeInfo *T, uint64_t val, bool isSigned)
    : ExprStmt(RValue, T),
        Value(T->ByteSize * 8, val, isSigned) 
{
    CHECK_EQ(8 % T->ByteSize, 0) << "IntegerLiteral supports 1, 2, 4, 8 bytes integer only!";
}

IntegerLiteral::IntegerLiteral(TypeInfo *T, llvm::StringRef valStr, uint8_t radix)
    : ExprStmt(RValue, T),
        Value(T->ByteSize * 8, valStr, radix)
{
    CHECK_EQ(8 % T->ByteSize, 0) << "IntegerLiteral supports 1, 2, 4, 8 bytes integer only!";
}

uint64_t IntegerLiteral::getVal() const {
    return Value.getZExtValue();
}

llvm::Value *IntegerLiteral::CodeGen(CompileUnitDecl *U) {
    return llvm::ConstantInt::get(*U->Context, Value);
}


//===-- FloatingLiteral --===//

FloatingLiteral::FloatingLiteral(TypeInfo *T, double val) 
    : ExprStmt(RValue, T),
        Value(val)
{
    assert(T->ByteSize * 8 == 64 && "Not a double Type T");
}

FloatingLiteral::FloatingLiteral(TypeInfo *T, float val)
    : ExprStmt(RValue, T),
        Value(val)
{   
    assert(T->ByteSize * 8 == 64 && "Not a double Type T");
}

double FloatingLiteral::getVal() const {
    return Value.convertToDouble();
}

llvm::Value *FloatingLiteral::CodeGen(CompileUnitDecl *U) {
    return llvm::ConstantFP::get(*U->Context, Value);
}