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
class DeclStmt;

/// \brief C-family declarations
class Decl {
    uint8_t SubClassID;
public:
    Decl() = default;
    Decl(llvm::StringRef name) 
        : name(name) { }
public:
    enum DeclTy : uint8_t {
        kUnkown,
    #define HANDLE_AST_DECL(X) k##X, 
    #include "AST/Decl.def"
        NUM_DECL
    };
public:
    unsigned getDeclID() const { return SubClassID; }
    llvm::StringRef getName() const { return name; }
protected:
    llvm::StringRef name;
};

/// \brief general variable declaration,
/// Bison parser cannot get VarDecl type.
class VarDecl : public Decl {
    uint8_t SubClassID { Decl::kVarDecl };
public:
    VarDecl() = default;
    VarDecl(llvm::StringRef name) 
        : Decl(name) { }
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
    /// \brief Return the type of declaration;
    /// if type is nullptr, means the bison parser
    /// can not assign type immediately, and we require 
    /// parent AST node to set type correctly.
    void getType();

    static bool classof(const Decl *D) {
        return D->getDeclID() == Decl::kVarDecl;
    }
private:
    // initialize expression.
    ExprStmt *init_expr;
    TypeInfo *type {nullptr};
    enum DefinitionKind definition_kind;
    enum InitializationStyle initialization_style;
protected:
    /// \brief This method can be only called by friend class
    /// to set type after Bison parser get type of declaration.
    void setType(TypeInfo *T) {
        type = T;
    }
    // friend classes.
    friend class ExprStmt;
    friend class DeclStmt;
};

/// \brief Function parameter declaration.
class ParamDecl : public VarDecl {
    uint8_t SubClassID { Decl::kParamDecl };
public:
    ParamDecl() = default;
    ParamDecl(TypeInfo *T, llvm::StringRef name) 
        : VarDecl(name) 
    {
        VarDecl::setType(T);
    }
    // getType() public inherit from VarDecl::getType().

    static bool classof(const Decl *D) {
        return D->getDeclID() == Decl::kParamDecl;
    }

};


/// \brief Function declaration.
class FunctionDecl : public Decl {
    uint8_t SubClassID { Decl::kFunctionDecl };
public:
    FunctionDecl() = default;
    FunctionDecl(TypeInfo *T, llvm::StringRef name) 
        : Decl(name) 
    {
        return_type = T;
    }
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

    static bool classof(const Decl *D) {
        return D->getDeclID() == Decl::kFunctionDecl;
    }
private:
    llvm::SmallVector<ParamDecl *, 10> param_list;
    // The body is usually a {} braced `CompoundStmt`.
    Stmt *Body;
    TypeInfo *return_type;
};

#endif