%code requires {
#include <memory>
#include <string>
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/TypeInfo.h"
#include "logging.h"
}

%{
#include <memory>
#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/TypeInfo.h"

class CompileUnitDecl;
class VarDecl;
class Stmt;
class DeclStmt;
class IfStmt;
class ForStmt;
class WhileStmt;
class CompoundStmt;
class ExprStmt;
class CallStmt;
class ArraySubscriptStmt;
class DeclRefStmt;
class BinaryOperatorStmt;
class IntegerLiteral;
class FloatingLiteral;
class StringLiteral;

using namespace std;

int yylex();
void yyerror(CompileUnitDecl& comp_unit, const char *s);
extern FILE* yyin;

static Decl *func_decl_tail;
static Decl *param_decl_tail;
static Stmt *expr_stmt_tail;
static Stmt *comp_stmt_tail;
static Decl *iden_tail;
%}

%parse-param { CompileUnitDecl& comp_unit }

%union {
  std::string *str_val;
  int int_val;
  TypeInfo* type_info_val;
  Stmt* stmt_val;
  Decl* decl_val;
  FunctionDecl* func_val;
  ParamDecl* param_val;
  VarDecl* ident_val;
  CompileUnitDecl* comp_val;
  IntegerLiteral* li_val;
  StringLiteral* st_val;
  DeclStmt* declstmt_val;
}

%type <str_val> basicType 
%type <func_val> FunctionDecl CompileUnit Function
%type <param_val> ParamList ParamDecl
%type <ident_val> IdentifierList  VarDecl
%type <comp_val> Program
%type <stmt_val> Stmt CompoundStmtList ReturnStmt ExprStmt Expr DeclRefStmt CallStmt ExprStmtList IfStmt MatchedStmt UnmatchedStmt WhileStmt Block Subscript SubscriptList
%type <li_val> IntegerLiteral
%type <st_val> StringLiteral
%type <declstmt_val> DeclStmt
%token T_CHAR T_INT T_STRING T_BOOL T_VOID
%token COMMA SEMICOLON OPENPAREN CLOSEPAREN OPENBRACE CLOSEBRACE OPENBRACKET CLOSEBRACKET
%token ADDR
%token TRUE FALSE
%token MOD BIAND BIOR NOT VOID
%token ASSIGN PLUSASSIGN MINUSASSIGN MULASSIGN DIVASSIGN
%token CONST IF ELSE WHILE BREAK CONTINUE RETURN
%token EQ GRAEQ LESEQ NEQ GRA LES
%token CHAR INTEGER STRING BOOLEAN DOUBLE
%token CAST
%token GENERIC_BEGIN GENERIC_END
%token <int_val> INT_CONST CharConst
%token <str_val> IDENTIFIER CONSTSTRING

%left EQ
%left OR XOR AND
%left PLUS MINUS
%left MUL DIV

%%
Program
: CompileUnit{
    comp_unit.CreateSubDecls($1);
}
;

CompileUnit
: CompileUnit FunctionDecl {
    func_decl_tail->Next = $2;
    func_decl_tail = $2;
    $$ = $1;
}
| FunctionDecl{
    auto func_decl =  $1;
    $1->Next = nullptr;
    func_decl_tail = func_decl;
    $$ = func_decl;
}
;

FunctionDecl
: Function Block{
    $1->setBody($2);
    $$ = $1;
}
| Function SEMICOLON {
    $$->setBody(nullptr);
    $$ = $1;
}

Function
: basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN {
    auto type = $1;
    auto ident = $2;
    auto param_list = $4;
    $$ = new FunctionDecl(TypeContext::find(*type), *ident, static_cast<ParamDecl*>(param_list));
}
| basicType IDENTIFIER OPENPAREN  CLOSEPAREN {
    auto type = $1;
    auto ident = $2;
    $$ = new FunctionDecl(TypeContext::find(*type), *ident, nullptr);
}
;

ParamList
: ParamDecl {
    $1->Next = nullptr;
    param_decl_tail = $1;
    $$ = $1;
}
| ParamList COMMA ParamDecl {
    $3->Next = nullptr;
    param_decl_tail->Next = $3;
    param_decl_tail = $3;
    $$ = $1;
}
;

ParamDecl
: basicType IDENTIFIER {
    auto type = $1;
    auto ident = $2;
    $$ = new ParamDecl(TypeContext::find(*type), *ident);
    $$->Next = nullptr;
}
| basicType IDENTIFIER OPENBRACKET CLOSEBRACKET{
    auto type = $1;
    auto ident = $2;
    $$ = new ParamDecl(REGISTER_POINTER(*type), *ident);
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET {
    auto type = $1;
    auto ident = $2;
    auto len = $4->getVal();
    $$ = new ParamDecl(REGISTER_ARRAY(*type, len), *ident);
}
; 

Block
: OPENBRACE CompoundStmtList CLOSEBRACE{
    auto comp_stmt = $2;
    $$ = new CompoundStmt(comp_stmt);
}
| OPENBRACE CLOSEBRACE {
    $$ = new CompoundStmt(nullptr);
}
;

basicType
: INTEGER {
    $$ = new string("int");
}
| CHAR {
    $$ = new string("char");
}
| VOID {
    $$ = new string("void");
}
| BOOLEAN {
    $$ = new string("bool");
}
| DOUBLE {
    $$ = new string("double");
}
;

CompoundStmtList
: CompoundStmtList Stmt {
    if ($1 != nullptr) {
        if ($2 != nullptr) {
            $1 -> Tail -> Next = $2;
            $1 -> Tail = $2;
            $2 -> Next = nullptr;
            $$ = $1;
        }
    }
    else {
        $$ = $2;
        $2->Tail = $2;
        $2->Next = nullptr;
    }
}
| Stmt {
    if ($1 != nullptr) {
        $$ = $1;
        $1->Tail = $1;
        $1->Next = nullptr;
    } else {
        $$ = nullptr;
    }
}

Stmt
: DeclStmt {
    $$ = $1;
}
| ReturnStmt {
    $$ = $1;
}
| IfStmt {
    $$ = $1;
}
| WhileStmt {
    $$ = $1;
}
| ExprStmt SEMICOLON {
    $$ = $1;
}
| SEMICOLON {
    $$ = nullptr;
}
;

ReturnStmt
: RETURN ExprStmt SEMICOLON {
    auto expr_stmt = $2;
    $$ = new ReturnStmt(static_cast<ExprStmt*>(expr_stmt));
    $$->Next = nullptr;
}
| RETURN SEMICOLON {
    $$ = new ReturnStmt(nullptr);
    $$->Next = nullptr;
}
;

IfStmt
: MatchedStmt{
    $$ = $1;
}
| UnmatchedStmt{
    $$ = $1;
}
;

MatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN Block ELSE Block{
    auto expr_stmt = $3;
    auto match_stmt = $5;
    auto smatch_stmt = $7;
    $$ = new IfStmt(static_cast<ExprStmt*>(expr_stmt), match_stmt, smatch_stmt);
}

UnmatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN Block{
    auto expr_stmt = $3;
    auto match_stmt = $5;
    $$ = new IfStmt(static_cast<ExprStmt*>(expr_stmt), match_stmt);
}
;

WhileStmt
: WHILE OPENPAREN ExprStmt CLOSEPAREN Block{
    auto expr_stmt = $3;
    auto comp_stmt = $5;
    $$ = new WhileStmt(static_cast<ExprStmt*>(expr_stmt), comp_stmt);
}

DeclStmt
: basicType IdentifierList SEMICOLON{
    auto type = $1;
    auto ident_list = $2;
    $$ = new DeclStmt(ident_list);
    $$ -> setType(TypeContext::find(*type));
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET SEMICOLON{
    auto type = $1;
    auto ident = $2;
    auto var = new VarDecl(*ident);
    var->Next = nullptr;
    var->setInit(nullptr);
    auto num = $4 -> getVal();
    $$ = new DeclStmt(var);
    $$ -> setType(TypeContext::createArrayType(*type, num));
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET OPENBRACKET IntegerLiteral CLOSEBRACKET SEMICOLON{
    auto type = $1;
    auto ident = $2;
    auto var = new VarDecl(*ident);
    var->Next = nullptr;
    var->setInit(nullptr);
    auto num_1 = $4 -> getVal();
    auto num_2 = $7 -> getVal();
    $$ = new DeclStmt(var);
    // Reverse Order.
    $$ -> setType(TypeContext::createArrayType(*type, {num_2, num_1}));
}
| basicType MUL IDENTIFIER SEMICOLON{
    auto type = $1;
    auto ident = $3;
    auto var = new VarDecl(*ident);
    var->Next = nullptr;
    var->setInit(nullptr);
    $$ = new DeclStmt(var);
    $$ -> setType(REGISTER_POINTER(*type));
}
;

ExprStmt
: Expr{
    $$ = $1;
}
| OPENPAREN Expr CLOSEPAREN{
    $$ = $2;
}
;

Expr
: ExprStmt ASSIGN ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Assign, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));    
}
|  ExprStmt OR ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Or, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}  
|  ExprStmt XOR ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Xor, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt AND ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::And, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt PLUS ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Add, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt MINUS ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Sub, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt MUL ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Mul, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt DIV ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Div, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
| ExprStmt EQ ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Equal, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt NEQ ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::NotEqual, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt GRA ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Greater, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt LES ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Less, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt GRAEQ ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::GreaterEqual, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
|  ExprStmt LESEQ ExprStmt{
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::LessEqual, static_cast<ExprStmt*>($1), static_cast<ExprStmt*>($3));  
}
| BIAND ExprStmt {
    $$ = new UnaryOperatorStmt(UnaryOperatorStmt::Addr, static_cast<ExprStmt*>($2));
}
| DeclRefStmt {
    $$ = $1;
}
| IntegerLiteral {
    $$ = $1;
}
| StringLiteral {
    $$ = $1;
}
| CallStmt {
    $$ = $1;
}
| CAST GENERIC_BEGIN basicType GENERIC_END OPENPAREN ExprStmt CLOSEPAREN {
    auto DestType = TypeContext::find(*$3);
    $$ = new Cast(static_cast<ExprStmt *>($6), DestType);
}
;

IntegerLiteral
: INT_CONST {
    auto IntType = TypeContext::find("int");
    auto num = $1;
    $$ = new IntegerLiteral(IntType, num);
}
| CharConst {
    auto CharType = TypeContext::find("char");
    auto num = $1;
    $$ = new IntegerLiteral(CharType, num);
}
| TRUE {
    auto BoolType = TypeContext::find("bool");
    $$ = new IntegerLiteral(BoolType, 1);
}
| FALSE {
    auto BoolType = TypeContext::find("bool");
    $$ = new IntegerLiteral(BoolType, 0);
}
;

StringLiteral
: CONSTSTRING {
    $$ = new StringLiteral(*$1);
};

DeclRefStmt
: IDENTIFIER SubscriptList {
    auto ident = $1;
    auto base = new DeclRefStmt(*ident);
    auto expr_stmt = $2;
    $$ = new ArraySubscriptStmt(base, static_cast<ExprStmt* >(expr_stmt));
}
| IDENTIFIER {
    auto ident = $1;
    $$ = new DeclRefStmt(*ident);
}

SubscriptList
: SubscriptList Subscript {
    $1->Next = $2;
    $1->Tail = $2;
    $$ = $1;
}
| Subscript {
    $$ = $1;
    $$->Tail = nullptr;
}

Subscript
: OPENBRACKET ExprStmt CLOSEBRACKET {
    $$ = $2;
}


CallStmt
: IDENTIFIER OPENPAREN ExprStmtList CLOSEPAREN{
    auto ident = $1;
    auto expr_list = $3;
    $$ = new CallStmt(*ident, static_cast<ExprStmt*>(expr_list));
}
;

ExprStmtList
: ExprStmt {
    auto expr_stmt = $1;
    expr_stmt_tail = expr_stmt;
    $$ = expr_stmt;
    $$->Next = nullptr;
}
| ExprStmtList COMMA ExprStmt{
    auto expr_stmt = $3;
    expr_stmt_tail -> Next = expr_stmt;
    expr_stmt_tail = $3;
    $$ = $1;
    $3->Next = nullptr;
}
;

IdentifierList
: VarDecl {
    iden_tail = $1;
    $$ = $1;
}
| IdentifierList COMMA VarDecl{
    iden_tail->Next = $3;
    iden_tail = $3;
    $$ = $1;
}
;

VarDecl
: IDENTIFIER {
    auto Var = new VarDecl(*$1);
    Var->Next = nullptr;
    Var->setInit(nullptr);
    $$ = Var;
}
| IDENTIFIER ASSIGN ExprStmt {
    auto Var = new VarDecl(*$1);
    Var->Next = nullptr;
    Var->setInit(static_cast<ExprStmt *>($3));
    $$ = Var;
}

%%

void yyerror(CompileUnitDecl& comp_unit, const char *s){
  cerr << "error: " << s << endl;
};
