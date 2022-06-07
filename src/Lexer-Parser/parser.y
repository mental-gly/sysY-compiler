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
  DeclStmt* declstmt_val;
}

%type <str_val> basicType 
%type <func_val> FunctionDecl CompileUnit
%type <param_val> ParamList ParamDecl
%type <ident_val> IdentifierList  VarDecl
%type <comp_val> Program
%type <stmt_val> Stmt CompoundStmtList ReturnStmt ExprStmt Expr DeclRefStmt CallStmt ExprStmtList IfStmt MatchedStmt UnmatchedStmt WhileStmt Block Subscript SubscriptList
%type <li_val> IntegerLiteral
%type <declstmt_val> DeclStmt
%token T_CHAR T_INT T_STRING T_BOOL T_VOID
%token COMMA SEMICOLON OPENPAREN CLOSEPAREN OPENBRACE CLOSEBRACE OPENBRACKET CLOSEBRACKET
%token ADDR
%token MOD BIAND BIOR NOT VOID
%token ASSIGN PLUSASSIGN MINUSASSIGN MULASSIGN DIVASSIGN
%token CONST IF ELSE WHILE BREAK CONTINUE RETURN
%token EQ GRAEQ LESEQ NEQ GRA LES
%token CHAR INTEGER STRING BOOLEAN
%token <int_val> INT_CONST
%token <str_val> IDENTIFIER 

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
    func_decl_tail = func_decl;
    $$ = func_decl;
    $$->Next = nullptr;
}
;

FunctionDecl 
: basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN Block{
    auto type = $1;
    auto ident = $2;
    auto param_list = $4;
    auto block = $6;
    LOG(INFO) << "Function "<< *ident;
    $$ = new FunctionDecl(TypeContext::find(*type), *ident, static_cast<ParamDecl*>(param_list));
    $$ -> setBody(block);
}
| basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN SEMICOLON{
    auto type = $1;
    auto ident = $2;
    auto param_list = $4;
    $$ = new FunctionDecl(TypeContext::find(*type), *ident, static_cast<ParamDecl*>(param_list));
}
;

ParamList
: ParamDecl {
    $1->Next = nullptr;
    param_decl_tail = $1;
    $$ = $1;
}
| ParamList COMMA ParamDecl {
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
    LOG(INFO) << "ParamDecl " << *($2) << " '" << *($1) << "'";
}
| basicType IDENTIFIER OPENBRACKET CLOSEBRACKET{
    auto type = $1;
    auto ident = $2;
    $$ = new ParamDecl(REGISTER_ARRAY(*type, -1), *ident);
    LOG(INFO) << "ParamDecl " << *($2) << " '" << *($1) << "'";
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
    LOG(INFO) << "Block with CompoundStmt list header :" << $2;
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
;

CompoundStmtList
: CompoundStmtList Stmt {
    if ($1 != nullptr) {
        if ($2 != nullptr) {
            $1 -> Tail -> Next = $2;
            $1 -> Tail = $2;
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
    auto comp_ptr = $1;
    $$ = comp_ptr;
    $$ -> Tail = comp_ptr;
    LOG(INFO) << "DeclStmt as CompoundStmt list header : " << $$;
}
| ReturnStmt {
    auto comp_ptr = $1;
    $$ = comp_ptr;
    $$ -> Tail = comp_ptr;
}
| IfStmt {
    auto comp_ptr = $1;
    $$ = comp_ptr;
    $$ -> Tail = comp_ptr;
    LOG(INFO) << "Add IF to CompoundStmt tail";
}
| WhileStmt {
    auto comp_ptr = $1;
    $$ = comp_ptr;
    $$ -> Tail = comp_ptr;
}
| ExprStmt SEMICOLON {
    auto comp_ptr = $1;
    $$ = comp_ptr;
    $$ -> Tail = comp_ptr;
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
    LOG(INFO) << "Matched IF";
}

UnmatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN Block{
    auto expr_stmt = $3;
    auto match_stmt = $5;
    $$ = new IfStmt(static_cast<ExprStmt*>(expr_stmt), match_stmt);
    LOG(INFO) << "Unmatched IF";
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
    auto num = $4 -> getVal();
    $$ = new DeclStmt(var);
    $$ -> setType(REGISTER_ARRAY(*type, num));
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
: DeclRefStmt ASSIGN ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Assign, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));    
}
| DeclRefStmt OR ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Or, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}  
| DeclRefStmt XOR ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Xor, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt AND ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::And, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt PLUS ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Add, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt MINUS ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Sub, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt MUL ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Mul, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt DIV ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Div, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt EQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Equal, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt NEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::NotEqual, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt GRA ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Greater, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt LES ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Less, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt GRAEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::GreaterEqual, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| DeclRefStmt LESEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::LessEqual, static_cast<ExprStmt*>(ref_stmt), static_cast<ExprStmt*>(expr_stmt));  
}
| IntegerLiteral OR ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Or, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));  
}  
| IntegerLiteral XOR ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Xor, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral AND ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::And, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral PLUS ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Add, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral MINUS ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Sub, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral MUL ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Mul, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral DIV ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Div, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral EQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Equal, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral NEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::NotEqual, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral GRA ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Greater, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral LES ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Less, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral GRAEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::GreaterEqual, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| IntegerLiteral LESEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::LessEqual, static_cast<ExprStmt*>(int_li), static_cast<ExprStmt*>(expr_stmt));
}
| DeclRefStmt {
    $$ = $1;
}
| IntegerLiteral {
    $$ = $1;
}
| CallStmt {
    $$ = $1;
}
;

IntegerLiteral
: INT_CONST {
    auto IntType = TypeContext::find("int");
    auto num = $1;
    $$ = new IntegerLiteral(IntType, num);
}
;

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
}
| ExprStmtList COMMA ExprStmt{
    auto expr_stmt = $3;
    expr_stmt_tail -> Next = expr_stmt;
    expr_stmt -> Prev = expr_stmt_tail;
    expr_stmt_tail = expr_stmt_tail -> Next;
    $$ = $1;
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
