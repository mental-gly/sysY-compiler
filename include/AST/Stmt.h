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
#include "AST/Decl.h"
#include "logging.h"
#include <assert.h>
#include <cstddef>
#include <memory>
#include <vector>
#include <list>


/// \brief Statement AST node (expression included)
class Stmt {
    uint8_t SubClassID;
public:
    virtual ~Stmt() = default;
public:
    // statement types.
    enum StmtTy : uint8_t {
        kUnknown,
    #define HANDLE_AST_STMT(X) k##X,
    #define HANDLE_AST_STMT_EXPR(X) k##X ,
    #include "AST/Stmt.def"
        NUM_STMTS
    };

    unsigned getStmtID() const {
        return SubClassID;
    }
public: 
    virtual llvm::Value *CodeGen() = 0;
};

/// \brief DeclStmt has a single or mutiple decls,
/// We assume child VarDecl has the same type.
/// \code int a, b, c=1; double d;
class DeclStmt : public Stmt {
    using DeclListTy = llvm::SmallVector<VarDecl *, 10>;
    uint8_t SubClassID { Stmt::kDeclStmt };
public:
    DeclStmt() = delete;
    DeclStmt(VarDecl *DeclList);
public:
    /// \brief After Bison parser get type,
    /// set type of child declaration list.
    void setType(TypeInfo *type);
    DeclListTy &getDeclList() { return decl_list; }
    llvm::Value *CodeGen() override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kDeclStmt;
    }
private:
    DeclListTy decl_list;
};





/// \brief curly braced statement block
class CompoundStmt : public Stmt {
    uint8_t SubClassID { Stmt::kCompoundStmt };
public:
    using StmtListType = llvm::ilist<Stmt *>;
public:
    const StmtListType &getStmtList() const;
    llvm::Value *CodeGen() override;
    /// \brief add sub statements of Compound statement.
    void CreateSubStmt(Stmt *);

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kCompoundStmt;
    }
private:
    StmtListType Stmts;
};


/// \brief expression (literals included);
/// expression has type checking.
class ExprStmt : public Stmt {
    uint8_t SubClassID { Stmt::kExprStmt };
public:
    // category expression values borrowing C++11 scheme.
    enum ExprValueKind : uint8_t {
        RValue,
        LValue,
    };
public:
    ExprStmt() = default;
    ExprStmt(enum ExprValueKind VK, TypeInfo *T = nullptr) {
        ValueKind = VK;
        ExprType = T;
    }
    ExprStmt(const ExprStmt &) = delete;
public:
    /// \brief Get the expression type;
    /// if null, calculate the expression type.
    /// The design of this function is owe to bottom up parsing 
    /// maynot get the type immediately, otherwise it's costly to
    /// modify the attribute grammar such that all attribute is 
    /// synthesis attribute.
    /// We may use hand-written top-down paresr to fix this problem.
    virtual TypeInfo *getType() = 0;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kExprStmt;
    }
protected:
    enum ExprValueKind ValueKind;
    TypeInfo *ExprType;
}; 


/// \brief Function call that may have a return value.
class CallStmt : public ExprStmt {
    uint8_t SubClassID { Stmt::kCallStmt };
public:
    CallStmt() = delete;
    CallStmt(llvm::StringRef func, ExprStmt *args);
public:
    llvm::Value *CodeGen() override;
    /// \brief Get the function call return type.
    TypeInfo *getType() override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kCallStmt;
    }
private:
    // Callee fucntion.
    llvm::StringRef Callee;
    llvm::SmallVector<ExprStmt *, 10> Args;
};


/// \brief Explicit showing the AST refer to a variable
/// given the symbol, DeclRef found the latest definition in LLVM IR SSA form.
class DeclRefStmt : public ExprStmt {
    uint8_t SubClassID;
public:
    DeclRefStmt() = delete;
    DeclRefStmt(llvm::StringRef symbol);
    llvm::Value *CodeGen() override;
    TypeInfo *getType() override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kDeclRefStmt;
    }
private:
    Stmt *decl;
    llvm::StringRef symbol;
};

/// \brief Array subscipt reference with the type A[4];
/// 'A' is Base and '4' is Idx.
/// \example A[4] = 1;
class ArraySubscriptStmt : public ExprStmt {
public:
    ArraySubscriptStmt() = delete;
    ArraySubscriptStmt(ExprStmt *Base, ExprStmt *Idx);
    llvm::Value *CodeGen() override;
    /// \brief Get the type of array subscription like 
    /// \example for int A[]; A[4] is a 'int'.
    TypeInfo *getType() override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kArraySubscriptStmt;
    }
private:
    ExprStmt *base, *idx;
};


/// \brief Return statement; `return;` for void function or
/// `return xxx;` for non-void function.
class ReturnStmt : public Stmt {
    uint8_t SubClassID { Stmt::kReturnStmt };
public:
    /// \brief Return expression, nullptr for void;
    ReturnStmt(ExprStmt * ReturnExpr);

public:
    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kReturnStmt;
    }

private:
    ExprStmt *value;
};


/// \brief Binary operations like plus, minus in AST.
class BinaryOperator : public ExprStmt {
    uint8_t SubClassID { Stmt::kBinaryOperator };
public:
    /// \brief Binary Operator varieties
    enum BinaryOpcode : uint8_t {
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
        : ExprStmt(ExprStmt::RValue), Opcode(opcode), SubExprs({LHS, RHS}) { }
public:
    uint32_t getOpcode() const { return Opcode; }
    llvm::Value *CodeGen() override;
    TypeInfo *getType() override;
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kBinaryOperator;
    }
protected:
    llvm::Value *Operands[2];
    Stmt *SubExprs[2];
    enum BinaryOpcode Opcode {Uknown};
};


/// \brief Integer literals in AST.
class IntegerLiteral : public ExprStmt {
    uint8_t SubClassID { Stmt::kIntegerLiteral };
public:
    IntegerLiteral() = delete;
    /// \brief Use integer variable to construct a IntegerLiteral
    IntegerLiteral(TypeInfo *T, uint64_t val, bool isSigned=false);
    /// \brief Use parsed string to construct a IntegerLiteral
    IntegerLiteral(TypeInfo *T, llvm::StringRef valStr, uint8_t radix);
public:
    /// \brief Get the value of literal.
    uint64_t getVal();
    llvm::Value *CodeGen() override;
    TypeInfo *getType() override;
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kIntegerLiteral;
    } 
protected:
    llvm::APInt Value;
    llvm::IntegerType *Type;
};


/// \brief floating point number literals in AST.
class FloatingLiteral : public ExprStmt {
    uint8_t SubClassID { Stmt::kFloatingLiteral };
public:
    FloatingLiteral() = delete;
    // Constructor of double literal.
    FloatingLiteral(TypeInfo *T, double val);
    // a variant of float literal.
    FloatingLiteral(TypeInfo *T, float val); 
public:
    llvm::Value *CodeGen() override;
    TypeInfo *getType() override;
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kFloatingLiteral;
    }
protected:
    // llvm::APFloat support convertXXX for casting.
    // llvm::IEEEFloat also works
    llvm::APFloat Value;
    llvm::Type *Type;
};

#endif // AST_H 