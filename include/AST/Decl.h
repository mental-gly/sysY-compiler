#ifndef DECL_H
#define DECL_H

/// \file Decl.h
/// \brief AST declaration node
/// \example 

#include <cstddef>
#include <cstdint>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/ilist.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include "TypeInfo.h"


class Stmt;
class ExprStmt;

/// \brief C-family declarations
class Decl {
public:
    Decl() = default;
    Decl(uint32_t K, llvm::StringRef name) 
        : Kind(K), name(name) { }
public:
    enum : uint32_t {
        kUnkown,
        kFunctionDecl,
        kVarDecl,
        kParamDecl,
        kTypedecl, // typedef 
        NUM_DECL
    };
public:
    uint32_t getKind() const { return Kind; }
    llvm::StringRef getName() const { return name; }
    TypeInfo *getType() const { return type; }
protected:
    uint32_t Kind {kUnkown};
    llvm::StringRef name;
};

/// \brief general variable declaration,
/// Bison parser cannot get VarDecl type.
class VarDecl : public Decl {
public:
    VarDecl() = default;
    VarDecl(uint32_t K, llvm::StringRef name) 
        : Decl(K, name) { }
public:
    enum DefinitionKind : uint32_t {
        DeclarationOnly,
        Definition
    };
    enum InitializationStyle : uint32_t {
        CInit,
        CallInit,
        ListInit
    };
public:
    // set init expression
    void setInit(ExprStmt *);
    ExprStmt *getInit() const;
    void setInitStyle();
    DefinitionKind hasDefinition() const;
    void getType();
protected:
    // initialize expression.
    ExprStmt *init_expr;
    TypeInfo *type {nullptr};
    enum DefinitionKind definition_kind;
    enum InitializationStyle initialization_style;
};

/// \brief Function parameter declaration.
class ParamDecl : public VarDecl {
public:
    ParamDecl() = default;
    ParamDecl(uint32_t K, TypeInfo *T, llvm::StringRef name) 
        : VarDecl(K, T, name) { }
};


/// \brief Function declaration.
class FunctionDecl : public Decl {
public:
    FunctionDecl() = default;
    FunctionDecl(uint32_t K, TypeInfo *T, llvm::StringRef name) 
        : Decl(K, T, name) { }
public:
    // constructors
    FunctionDecl() = delete;
    /// \brief declare a function returns a value of \p return_type with no param.
    FunctionDecl(TypeInfo *return_type);
    /// \brief declare a function returns a value of \p return_type with parameters of \p params.
    FunctionDecl(TypeInfo *return_type, llvm::SmallVectorImpl<ParamDecl*> params);
public:
    /// \brief Returns true if the function has a function definition body.
    bool hasBody() const;
    /// \brief Returns true if the function has somewhere a definiyion
    bool hasDefiniton() const;
    void setBody(Stmt *B);
    TypeInfo *getReturnType() const;
    ParamDecl *getParams() const;

    llvm::Function* CodeGen();
private:
    llvm::SmallVector<ParamDecl *, 10> param_list;
    // The body is usually a {} braced `CompoundStmt`.
    Stmt *Body;
    TypeInfo *return_type;
};

#endif