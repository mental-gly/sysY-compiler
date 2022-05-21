#include "AST/Stmt.h"
#include "llvm/IR/ValueSymbolTable.h"
using namespace llvm;

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

Value *DeclStmt::CodeGen(CompileUnitDecl *U) {
    // currently do nothing.
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

Value *CompoundStmt::CodeGen(CompileUnitDecl *U) {
    // Compoundstmt inside is a new scope,
    // Which is managed by CompoundStmt.
    U->Symbol.CreateScope();
    ReturnInst *RetInstTrack;
    for (auto SubStmt : Stmts) {
        SubStmt->CodeGen(U);
        // optimization : ignore statements after ret.
        if (!ReturnStmt::classof(SubStmt)) break;
    }
    U->Symbol.LeaveScope();
    return nullptr;
}

//===-- IfStmt --===//

IfStmt::IfStmt(ExprStmt *Cond, Stmt *Then, Stmt *Else)
    : Cond(Cond), Then(Then), Else(Else)
{
    CHECK_NE(Cond, nullptr) << "Expect expression in `if` condition.";
    if (Then == nullptr)
        LOG(WARNING) << "Empty statement body.";
}

Value *IfStmt::CodeGen(CompileUnitDecl *U) {
    auto builder = U->getBuilder();
    auto context = U->getContext();
    Value *CondV = Cond->CodeGen(U);
    Value *LogicCondV;
    CHECK(CondV->getType()->isSingleValueType())
        << "Expect a scalar type in `if` condition";
    // If integer number, compare with 0.
    if (CondV->getType()->isIntegerTy()) {
        LogicCondV = builder->
                CreateICmpNE(CondV,
                             ConstantInt::get(CondV->getType(), 0),
                             "ifcond");
    }
    // If floating number, compare with 0.0.
    else if (CondV->getType()->isFloatTy()) {
        LogicCondV = builder->
                CreateFCmpONE(CondV,
                              ConstantFP::get(CondV->getType(), 0.0),
                              "ifcond");
    }
    // If pointer type, compare with null.
    else if (CondV->getType()->isPointerTy()) {
        LogicCondV = builder->
                CreateICmpNE(CondV,
                             ConstantPointerNull::get(
                                     dyn_cast<PointerType>(CondV->getType())),
                             "ifcond");
    }
    // Optimization, neglect empty then block.
    if (Then != nullptr) {
        // Get the parent function.
        auto F = builder->GetInsertBlock()->getParent();
        auto ThenBB = BasicBlock::Create(*context, "then", F);
        auto ElseBB = BasicBlock::Create(*context, "else");
        auto MergeBB = BasicBlock::Create(*context, "merge");
        // Conditional branch.
        builder->CreateCondBr(LogicCondV, ThenBB, ElseBB);
        // Gen Then.
        builder->SetInsertPoint(ThenBB);
        Then->CodeGen(U);
        ThenBB = builder->GetInsertBlock();
        if (!isa<ReturnInst>(ThenBB->back())) builder->CreateBr(MergeBB);
        // Update Then block;
        // if then block generate multiple BB,
        // we should locate the ThenBB to the last basic block.
        F->getBasicBlockList().push_back(ElseBB);
        // Gen Else.
        builder->SetInsertPoint(ElseBB);
        if (Else != nullptr) Else->CodeGen(U);
        ElseBB = builder->GetInsertBlock();
        if (!isa<ReturnInst>(ElseBB->back())) builder->CreateBr(MergeBB);
        F->getBasicBlockList().push_back(MergeBB);
        builder->SetInsertPoint(MergeBB);
    }
    return nullptr;
}



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
    return nullptr;
}

Value *CallStmt::CodeGen(CompileUnitDecl *U) {
    auto module = U->getModule();
    auto builder = U->getBuilder();
    llvm::Function *F = module->getFunction(Callee);
    CHECK(F) << "Call unknown function " << Callee;
    llvm::SmallVector<llvm::Value *, 10> ArgsV;
    for (ExprStmt *&Expr : Args) {
      ArgsV.push_back(Expr->CodeGen(U));
    }
    return builder->CreateCall(F, ArgsV, llvm::Twine("Call", Callee));
}

//===-- DeclRefStmt --===//

DeclRefStmt::DeclRefStmt(const std::string &symbol)
    : ExprStmt(ExprStmt::LValue), Symbol(symbol) { }

TypeInfo *DeclRefStmt::getType(CompileUnitDecl *U) {
    // If we have searched symbol of the Ref.
    if (ExprType != nullptr)
        return ExprType;
    return nullptr;
}

/// \brief Load variable into virtual register.
Value *DeclRefStmt::CodeGen(CompileUnitDecl *U) {
    auto builder = U->getBuilder();
    // The referenced symbol must be an alloca addr or global value addr,
    auto F = builder->GetInsertBlock()->getParent();
    Value *AddrOrGlob = F->getValueSymbolTable()->lookup(Symbol);
    // Value *AddrOrGlob = U->Symbol.lookup(Symbol);
    CHECK_NE(AddrOrGlob, nullptr) << "Undeclared variable " << "'" << Symbol << "'";
    CHECK((dyn_cast<GlobalValue>(AddrOrGlob) != nullptr)
          || (dyn_cast<AllocaInst>(AddrOrGlob) != nullptr)) << "Not a local or global variable.";
    return AddrOrGlob;
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
    return nullptr;
}

Value *ArraySubscriptStmt::CodeGen(CompileUnitDecl *U) {
    auto builder = U->getBuilder();
    auto BaseVal = Base->CodeGen(U);
    auto IdxVal = Base->CodeGen(U);
    CHECK(BaseVal->getType()->isArrayTy() || BaseVal->getType()->isPointerTy())
        << "'" << reinterpret_cast<DeclRefStmt *>(BaseVal)->getSymbolName().str() << "'"
        << " should be an array or a pointer type";
    return builder->CreateGEP(BaseVal, IdxVal);
}

//===-- ReturnStmt --===//
Value *ReturnStmt::CodeGen(CompileUnitDecl *U) {
    // If the function has non-void ret value,
    // we alloca a space for ret value;
    // We will create an extra BasicBlock
    // which load the RetVal and do the final actual return.
    //
    // It's an alternative to add a final ret BB, and let
    // any internal return br to this final BB.
    auto builder = U->getBuilder();
    llvm::Value *RetV = RetExpr->CodeGen(U);
    builder->CreateRet(RetV);
    return nullptr;
}




//===-- BinaryOperator --===//

TypeInfo *BinaryOperatorStmt::getType(CompileUnitDecl *U) {
    if (ExprType != nullptr)
        return ExprType;
    return nullptr;
}

static Value *doGreater(IRBuilder<> *builder, Value *LHS, Value *RHS, bool isSigned) {
    // We suppose after semantic analysis or carefully
    // write code, LHS and RHS has the same type.
    if (LHS->getType()->isIntegerTy()) {
        auto LType = dyn_cast<IntegerType>(LHS->getType());
        auto RType = dyn_cast<IntegerType>(RHS->getType());
        if (isSigned) return builder->CreateICmpSGT(LHS, RHS);
        else return builder->CreateICmpUGT(LHS, RHS);
    }
    if (LHS->getType()->isDoubleTy() || LHS->getType()->isFloatTy()) {
        return builder->CreateFCmpOGT(LHS, RHS);
    }
    return nullptr;
}

Value *BinaryOperatorStmt::CodeGen(CompileUnitDecl *U) {
    auto builder = U->getBuilder();
    Operands[0] = SubExprs[0]->CodeGen(U);
    Operands[1] = SubExprs[1]->CodeGen(U);
    // We use load/store pair to store local variables,
    // which introduce a problem that weather it refer a addr/value
    // while using the name of a variable.
    //
    // Clang use ImplicitCastOp casting LValue to RValue to solve this problem.
    // Here we do somehow similar but a little dirty works.
    if (Opcode != Assign && SubExprs[0]->getValueKind() == ExprStmt::LValue)
        Operands[0] = builder->CreateLoad(Operands[0]->getType()->getPointerElementType(), Operands[0]);
    if (SubExprs[1]->getValueKind() == ExprStmt::LValue)
        Operands[1] = builder->CreateLoad(Operands[1]->getType()->getPointerElementType(),Operands[1]);
    if (Operands[0] == nullptr || Operands[1] == nullptr)
        return nullptr;
    // considering implicit casting
    if (Operands[0]->getType() != Operands[1]->getType()) {

    }
    // neglect Twine name
    CHECK_NE(Opcode, Unknown) << "Unknown Binary Operation Opcode!";
    switch (Opcode) {
        case Add    :   return builder->CreateAdd(Operands[0], Operands[1]);
        case Sub    :   return builder->CreateSub(Operands[0], Operands[1]);
        case Mul    :   return builder->CreateMul(Operands[0], Operands[1]);
        case Assign :   return builder->CreateStore(Operands[1], Operands[0]);
        case Greater : return doGreater(builder, Operands[0], Operands[1], isSigned);
    }
    return nullptr;
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

Value *IntegerLiteral::CodeGen(CompileUnitDecl *U) {
    return ConstantInt::get(*U->getContext(), Value);
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

Value *FloatingLiteral::CodeGen(CompileUnitDecl *U) {
    return ConstantFP::get(*U->getContext(), Value);
}