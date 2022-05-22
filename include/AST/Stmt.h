#ifndef AST_H
#define AST_H
/// \file AST.h
/// \brief statement type AST node
/// and two basic derived type StmtAST and DeclAST.
///
/// Refer to Clang AST Stmt & Decl designs.
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "ADT/ilist.h"
#include "AST/TypeInfo.h"
#include "AST/Decl.h"
#include "logging.h"
#include <assert.h>
#include <cstddef>
#include <memory>
#include <vector>
#include <list>

class CompileUnitDecl;
class VarDecl;
class Stmt;
class DeclStmt;
class IfStmt;
class ForStmt;
class CompoundStmt;
class ExprStmt;
class CallStmt;
class ArraySubscriptStmt;
class DeclRefStmt;
class BinaryOperatorStmt;
class IntegerLiteral;
class FloatingLiteral;

/// \brief Statement AST node (expression included)
class Stmt : public ast_ilist_node<Stmt> {
    uint8_t SubClassID;
public:
    Stmt() = default;
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
    virtual llvm::Value *CodeGen(CompileUnitDecl *) = 0;
#if !defined(NDEBUG)
    virtual void dump() = 0;
#endif
};

/// \brief DeclStmt has a single or multiple decls,
/// We assume child VarDecl has the same type.
/// \code int a, b, c=1; double d;
class DeclStmt final : public Stmt {
    using DeclListTy = llvm::SmallVectorImpl<VarDecl *>;
    uint8_t SubClassID { Stmt::kDeclStmt };
public:
    DeclStmt() = delete;
    DeclStmt(VarDecl *DeList);
public:
    /// \brief After Bison parser get type,
    /// set type of child declaration list.
    void setType(TypeInfo *type);
    DeclListTy &getDeclList() {
        return Decls;
    }
    llvm::Value *CodeGen(CompileUnitDecl *) override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kDeclStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    llvm::SmallVector<VarDecl *, 10> Decls;
};


/// \brief curly braced statement block
class CompoundStmt : public Stmt {
    uint8_t SubClassID { Stmt::kCompoundStmt };
public:
    using StmtListType = llvm::SmallVectorImpl<Stmt *>;
public:
    CompoundStmt() = default;
    CompoundStmt(Stmt *StList);

public:
    /// \brief Return the vector of statements in the compoundstmt.
    const StmtListType &getStmtList() const {
        return Stmts;
    }
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    /// \brief add sub statements of Compound statement.
    void CreateSubStmt(Stmt *);

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kCompoundStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    llvm::SmallVector<Stmt *, 10> Stmts;
};


class IfStmt : public Stmt {
    uint8_t SubClassID {Stmt::kIfStmt };
public:
    IfStmt() = delete;
    IfStmt(ExprStmt *Cond, Stmt *Then, Stmt *Else = nullptr);
public:
    llvm::Value *CodeGen(CompileUnitDecl *) override;
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    ExprStmt *Cond;
    Stmt *Then, *Else;
};


class WhileStmt : public Stmt {
    uint8_t  SubClassID { Stmt::kWhileStmt };
public:
    WhileStmt() = delete;
    WhileStmt(ExprStmt *Cond, Stmt *Body = nullptr);
public:
    bool hasBody() const { return Body != nullptr; }
    void setBody(Stmt *B) { Body = B; }
    llvm::Value *CodeGen(CompileUnitDecl *) override;
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    ExprStmt *Cond;
    Stmt *Body;
};

class ForStmt : public Stmt {
    uint8_t SubClassID { Stmt::kForStmt };
public:
    ForStmt() = delete;
    ForStmt(Stmt *Init, ExprStmt *Cond, ExprStmt *Inc, Stmt *Body);
private:
    Stmt *Init;
    ExprStmt *Cond;
    ExprStmt *Inc;
    Stmt *Body;
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
    explicit ExprStmt(enum ExprValueKind VK, TypeInfo *T = nullptr) {
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
    virtual TypeInfo *getType(CompileUnitDecl *) = 0;
    enum ExprValueKind getValueKind() const { return ValueKind; }
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
    CallStmt(const std::string &Func, ExprStmt *ArgList = nullptr);
public:
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    /// \brief Get the function call return type.
    TypeInfo *getType(CompileUnitDecl *) override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kCallStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    // Callee function name.
    std::string Callee;
    llvm::SmallVector<ExprStmt *, 10> Args;
};


/// \brief Explicit showing the AST refer to a variable
/// given the symbol.
///
/// Now the DeclRefStmt can not necessarily track the definition
/// if we use alloca, load/store to get access to a variable.
class DeclRefStmt : public ExprStmt {
    uint8_t SubClassID;
public:
    DeclRefStmt() = delete;
    DeclRefStmt(const std::string &symbol);
    llvm::StringRef getSymbolName() const {
        return Symbol;
    }
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    TypeInfo *getType(CompileUnitDecl *) override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kDeclRefStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    std::string Symbol;
};

/// \brief Array subscript reference with the type A[4];
/// 'A' is Base and '4' is Idx.
/// \example A[4] = 1;
class ArraySubscriptStmt : public ExprStmt {
public:
    ArraySubscriptStmt() = delete;
    ArraySubscriptStmt(ExprStmt *base, ExprStmt *idx);
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    /// \brief Get the type of array subscription like 
    /// \example for int A[]; A[4] is a 'int'.
    TypeInfo *getType(CompileUnitDecl *) override;

    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kArraySubscriptStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    ExprStmt *Base, *Idx;
};


/// \brief Return statement; `return;` for void function or
/// `return xxx;` for non-void function.
class ReturnStmt : public Stmt {
    uint8_t SubClassID { Stmt::kReturnStmt };
public:
    /// \brief Return expression, nullptr for void;
    ReturnStmt(ExprStmt * RetExpr) : RetExpr(RetExpr) { }

public:
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    static bool classof(Stmt *S) {
        return S->getStmtID() == Stmt::kReturnStmt;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    ExprStmt *RetExpr;
};


/// \brief Binary operations like plus, minus in AST.
class BinaryOperatorStmt : public ExprStmt {
    uint8_t SubClassID { Stmt::kBinaryOperator };
public:
    /// \brief Binary Operator varieties
    enum BinaryOpcode : uint8_t {
        Unknown,
    #define BINARY_OPERATOR(X) X,
    #include "Expression.def"
        NUM_BINARY_OPERATORS,
    };
public:
    // constructors.
    BinaryOperatorStmt() = delete;
    // If given two operands, the type of result need to deduce later.
    BinaryOperatorStmt(enum BinaryOpcode opcode, ExprStmt *LHS, ExprStmt *RHS)
        : ExprStmt(ExprStmt::RValue), Opcode(opcode)
    {
      SubExprs[0] = LHS;
      SubExprs[1] = RHS;
    }
public:
    uint32_t getOpcode() const { return Opcode; }
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    TypeInfo *getType(CompileUnitDecl *) override;
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kBinaryOperator;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
protected:
    llvm::Value *Operands[2];
    ExprStmt *SubExprs[2];
    enum BinaryOpcode Opcode {Unknown};
    bool isSigned {true};
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
    uint64_t getVal() const;
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    TypeInfo *getType(CompileUnitDecl *U) override {
        return ExprType;
    }
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kIntegerLiteral;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
protected:
    llvm::APInt Value;
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
    double getVal() const;
    llvm::Value *CodeGen(CompileUnitDecl *) override;
    TypeInfo *getType(CompileUnitDecl *U) override {
        return ExprType;
    }
    static bool classof(const Stmt *S) {
        return S->getStmtID() == Stmt::kFloatingLiteral;
    }
#if !defined(NDEBUG)
    void dump() override;
#endif
protected:
    // llvm::APFloat support convertXXX for casting.
    // llvm::IEEEFloat also works
    llvm::APFloat Value;
};


class StringLiteral : public ExprStmt {
    uint8_t SubClassID { Stmt::kStringLiteral };
public:
    StringLiteral() = delete;
    StringLiteral(const std::string);
public:
    TypeInfo *getType(CompileUnitDecl *U) override {
        return TypeContext::find("char*");
    }
    llvm::StringRef getVal() const { return Literal; }
    llvm::Value *CodeGen(CompileUnitDecl *) override;
#if !defined(NDEBUG)
    void dump() override;
#endif
private:
    std::string Literal;
};

#endif // AST_H 