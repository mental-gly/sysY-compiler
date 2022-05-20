#ifndef COMPILER_DECL_H
#define COMPILER_DECL_H

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
#include <llvm/IR/IRBuilder.h>
#include "ADT/ilist.h"
#include "AST/TypeInfo.h"
#include "AST/SymbolTable.h"
#include "AST/Stmt.h"


/// \brief C-family declarations
class Decl : public ast_ilist_node<Decl> {
    uint8_t SubClassID;
public:
    Decl() = default;
    Decl(const std::string &name)
        : Name(name) { }
public:
    enum DeclTy : uint8_t {
        kUnknown,
    #define HANDLE_AST_DECL(X) k##X, 
    #include "AST/Decl.def"
        NUM_DECL
    };
public:
    unsigned getDeclID() const { return SubClassID; }
    llvm::StringRef getName() const { return Name; }
protected:
    const std::string &Name;
};


/// \brief CompileUnit is a helper collect global
/// declarations and build LLVM IR.
class CompileUnitDecl final : public Decl {
    uint8_t SubClassID { Decl::kCompileUnitDecl };
public:
    CompileUnitDecl() = delete;
    /// \brief We expect a compile unit is composed of
    /// a list of declarations; nullptr if there is no
    /// declarations
    CompileUnitDecl(Decl *DeList, const std::string &FileName);
public:
    /// \brief Dump the CompileUnit.
    void print() const { Module->print(llvm::outs(), nullptr); };
    void CodeGen();
public:
    llvm::SmallVector<Decl *, 10> Decls;
    std::unique_ptr<llvm::LLVMContext> Context;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::unique_ptr<llvm::Module> Module;
    SymbolTable<llvm::Value *> Symbol;
};


/// \brief general variable declaration,
/// Bison parser cannot get VarDecl type.
///
/// multi-inherit to let VarDecl linked together.
class VarDecl : public Decl {
    uint8_t SubClassID { Decl::kVarDecl };
public:
    VarDecl() = default;
    VarDecl(const std::string &name)
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
    void setInit(ExprStmt *Init) { init_expr = Init; }
    // get init expression
    ExprStmt *getInit() const { return init_expr; }
    void setInitStyle(enum InitializationStyle InitStyle) {
        initialization_style = InitStyle;
    }
    enum InitializationStyle getInitStyle() const {
        return initialization_style;
    }
    /// \brief Return the type of declaration;
    /// if type is nullptr, means the bison parser
    /// can not assign type immediately, and we require 
    /// parent AST node to set type correctly.
    TypeInfo *getType() const { return type; }

    /// \brief Allocate space for Local variable or
    /// declare global variable
    llvm::Value *CodeGen(CompileUnitDecl *);

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
    ParamDecl(TypeInfo *T, const std::string &name)
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
class FunctionDecl final : public Decl {
    uint8_t SubClassID { Decl::kFunctionDecl };
public:
    // constructors
    FunctionDecl() = delete;
    /// \brief declare a function returns a value of \p return_type with no param.
    FunctionDecl(TypeInfo *return_type, const std::string &name)
        : Decl(name) { ReturnType = return_type; }
    /// \brief declare a function returns a value of \p return_type with parameters of \p params.
    FunctionDecl(TypeInfo *return_type, const std::string &name, ParamDecl *ParaList);
public:
    llvm::Function* CodeGen(CompileUnitDecl *);
    /// \brief Returns true if the function has a function definition body.
    bool hasBody() const { return Body != nullptr; }
    void setBody(Stmt *B) { Body = B; }
    /// \brief Returns true if the function has somewhere a definition
    bool hasDefinition(CompileUnitDecl *) const;
    TypeInfo *getReturnType() const {
        return ReturnType;
    }

    llvm::SmallVectorImpl<ParamDecl *> &getParams() {
        return Params;
    }

    static bool classof(const Decl *D) {
        return D->getDeclID() == Decl::kFunctionDecl;
    }
private:
    llvm::SmallVector<ParamDecl *, 10> Params;
    // The body is usually a {} braced `CompoundStmt`.
    Stmt *Body;
    TypeInfo *ReturnType;
};

#endif // COMPILER_DECL_H