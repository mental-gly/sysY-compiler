#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/TypeInfo.h"

/// \file Analysis.cc
/// \brief Semantic Analysis

using namespace llvm;

static void StoreErrorMessages(uint8_t ErrorCode, const std::string &Message);

static void LRValueCheck(ExprStmt *Expr) {
    if (BinaryOperatorStmt::classof(Expr)) {
        auto BinaryExpr = static_cast<BinaryOperatorStmt *>(Expr);
        auto Opcode = BinaryExpr->getOpcode();
        if (Opcode == BinaryOperatorStmt::Assign) {
            auto LHS = BinaryExpr->getOperand(0);
            if (LHS->getValueKind() != ExprStmt::LValue) {
                //
            }
        }
    }
    if (UnaryOperatorStmt::classof(Expr)) {
        auto UnaryExpr = static_cast<UnaryOperatorStmt *>(Expr);
        auto Opcode = UnaryExpr->getOpcode();
        if (Opcode == UnaryOperatorStmt::Addr) {
            auto LHS = UnaryExpr->getOperand(0);
            if (LHS->getValueKind() != ExprStmt::LValue) {
                //
            }
        }
    }
}

static void VariableDeclarationCheckSubScope(CompoundStmt *Block, SymbolTable<TypeInfo *> ST) {
    ST.CreateScope();
    auto Stmts = Block->getStmts();
    for (const auto &SubStmt : Stmts) {
        if (CompoundStmt::classof(SubStmt)) {
            auto SubScope = static_cast<CompoundStmt *>(SubStmt);
            VariableDeclarationCheckSubScope(SubScope, ST);
        }
    }
    ST.LeaveScope();
}

static void VariableDeclarationCheck(CompileUnitDecl *U) {
    SymbolTable<TypeInfo *> ST;
    auto Decls = U->getDecls();
    for (auto &SubDecl : Decls) {
        if (FunctionDecl::classof(SubDecl)) {
            auto F = static_cast<FunctionDecl *>(SubDecl);
            auto Body = static_cast<CompoundStmt *>(F->getBody());
            VariableDeclarationCheckSubScope(Body, ST);
        }
    }
}

