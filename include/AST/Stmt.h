#ifndef AST_H
#define AST_H
/// \file AST.h
/// \brief delcare common ancenstor AST node 
/// and two basic derived type StmtAST and DeclAST.
///
/// We utilize a common ancenstor for AST node classes 
/// but refer to Clang AST Stmt & Decl designs.
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/APFixedPoint.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "AST/TypeInfo.h"
#include "logging.h"
#include <assert.h>
#include <cstddef>
#include <memory>
#include <vector>
#include <list>

class Decl;
class VarDecl;

/// \brief Statement AST node (expression included)
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual llvm::Value *CodeGen() = 0;
};

/// \brief DeclStmt has a single or mutiple decls
/// \code int a, b, c=1; double d;
class DeclStmt : public Stmt {
    using DeclListTy = llvm::SmallVector<VarDecl *, 10>;
public:
    DeclListTy &getDeclList() { return decl_list; }
    void setDeclList(llvm::SmallVectorImpl<VarDecl *> &decls);
    llvm::Value *CodeGen() override;
private:
    DeclListTy decl_list;
};


/// \brief Function call.
class CallStmt : public Stmt {
public:
    llvm::Value *CodeGen() override;
    TypeInfo *getReturnType();
private:
    // Callee fucntion.
    llvm::StringRef Callee;
    llvm::SmallVector<ExprStmt *, 10> Args;
};


/// \brief curly braced statement block
class CompoundStmt : public Stmt {
public:
    using StmtListType = llvm::ilist<Stmt *>;
public:
    const StmtListType &getStmtList() const;
    llvm::Value *CodeGen() override;
    /// \brief add sub statements of Compound statement.
    void CreateSubStmt(Stmt *);
private:
    StmtListType Stmts;
};


/// \brief expression (literals included).
class ExprStmt : public Stmt {
public:
    // category expression values borrowing C++11 scheme.
    enum ExprValueKind : uint32_t {
        RValue,
        LValue,
    };
public:
    ExprStmt() = default;
    ExprStmt(TypeInfo *T, enum ExprValueKind VK) {
        ExprType = T;
        ValueKind = VK;
    }
    ExprStmt(const ExprStmt &) = delete;

protected:
    TypeInfo *ExprType;
    enum ExprValueKind ValueKind;
}; 


/// \brief Binary operations like plus, minus in AST.
class BinaryOperator : public ExprStmt {
public:
    /// \brief Binary Operator varieties
    enum BinaryOpcode : uint32_t {
        Uknown,
    #define BINARY_OPERATOR(X) X,
    #include "Expression.def"
        NUM_BINARY_OPERATORS,
    };
public:
    // constructors.
    BinaryOperator() = delete;
    // If given two operands, the type of result need to deduce later.
    BinaryOperator(enum BinaryOpcode opcode, Stmt *LHS, Stmt *RHS)
        : Opcode(opcode), SubExprs({LHS, RHS}) { }
public:
    uint32_t getOpcode() const { return Opcode; }
    llvm::Value *CodeGen() override;
protected:
    llvm::Value *Operands[2];
    Stmt *SubExprs[2];
    enum BinaryOpcode Opcode {Uknown};
};


/// \brief Integer literals in AST.
class IntegerLiteral : public ExprStmt {
public:
    IntegerLiteral() = delete;
    /// \brief Use integer variable to construct a IntegerLiteral
    IntegerLiteral(TypeInfo *T, uint64_t val, bool isSigned=false);
    /// \brief Use parsed string to construct a IntegerLiteral
    IntegerLiteral(TypeInfo *T, llvm::StringRef valStr, uint8_t radix);
public:
    llvm::Value *CodeGen() override;
protected:
    llvm::APInt Value;
    llvm::IntegerType *Type;
};


/// \brief floating point number literals in AST.
class FloatingLiteral : public ExprStmt {
public:
    FloatingLiteral() = delete;
    // Constructor of double literal.
    FloatingLiteral(TypeInfo *T, double val);
    // a variant of float literal.
    FloatingLiteral(TypeInfo *T, float val); 
public:
    llvm::Value *CodeGen() override;
protected:
    // llvm::APFloat support convertXXX for casting.
    // llvm::IEEEFloat also works
    llvm::APFloat Value;
    llvm::Type *Type;
};

#endif // AST_H 