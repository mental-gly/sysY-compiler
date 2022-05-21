%code requires {
  #include <memory>
  #include <string>
}

%{
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "logging.h"
#include "AST/Decl.h"
#include "AST/Stmt.h"

using namespace std;

int yylex();
void yyerror(CompileUnitDecl& comp_unit, const char *s);
extern FILE* yyin;

FunctionDecl *func_decl_tail;
ParamDecl *param_decl_tail;
ExprStmt *expr_stmt_tail;
CompoundStmt *comp_stmt_tail;
VarDecl *iden_tail;
%}

%parse-param { CompileUnitDecl& comp_unit }

%union {
  std::string *str_val;
  int int_val;
  TypeInfo* type_info_val;
  Stmt* stmt_val;
  Decl* decl_val;
}

%type <type_info_val> basicType 
%type <decl_val> FunctionDecl ParamList ParamDecl IdentifierList CompileUnit Program
%type <stmt_val> CompoundStmtList DeclStmt ReturnStmt ExprStmt DeclRefStmt IntegerLiteral CallStmt ExprStmtList IfStmt MatchedStmt UnmatchedStmt WhileStmt Block
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
: CompileUnit FunctionDecl{
    auto func_decl = make_unique<FunctionDecl>($2);
    func_decl_tail -> Next = func_decl;
    func_decl -> Prev = func_decl_tail;
    func_decl_tail = func_decl_tail -> Next;
    $$ = $1;
}
| FunctionDecl{
    auto func_decl = make_unique<FunctionDecl>($1);
    func_decl_tail = func_decl;
    $$ = func_decl;
}
;

FunctionDecl 
: basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN Block{
    auto type = $1;
    auto ident = new string($2);
    auto param_list = $4;
    auto block = $6;
    $$ = new FunctionDecl(type, *ident, param_list);
    $$ -> setBody(block);
} 
| basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN SEMICOLON{
    auto type = $1;
    auto ident = new string($2);
    auto param_list = $4;
    $$ = new FunctionDecl(type, *ident, param_list);
}
;

ParamList
: ParamDecl{
    auto param_ptr = $1;
    param_decl_tail = param_ptr;
    $$ = param_ptr;  
}
| ParamList COMMA ParamDecl{
    auto param_decl = $3;
    param_decl_tail -> Next = param_decl;
    param_decl -> Prev = param_decl_tail;
    param_decl_tail = param_decl_tail -> Next;
    $$ = $1;  
}
;

ParamDecl
: {
    $$ = new ParamDecl();
}
|basicType IDENTIFIER{
    auto type = $1;
    auto ident = new string($2);
    $$ = new ParamDecl(type, *ident);
}
; 

Block
: OPENBRACE CompoundStmtList CLOSEBRACE{
    auto comp_stmt = $2;
    $$ = new CompoundStmt(comp_stmt);
}
;

basicType
: INTEGER {
    auto IntType = TypeContext::find("int");
    $$ = IntType;
}
| CHAR {
    auto CharType = TypeContext::find("bool");
    $$ = CharType;
}
| VOID {
    auto VoidType = TypeContext::find("void");
    $$ = VoidType;
}
;

CompoundStmtList
: CompoundStmtList DeclStmt {
    auto decl_stmt = $2;
    comp_stmt_tail -> Next = decl_stmt;
    decl_stmt -> Prev = comp_stmt_tail;
    comp_stmt_tail = comp_stmt_tail -> Next;
    $$ = $1;
}
| DeclStmt {
    auto comp_ptr = $1;
    comp_stmt_tail = comp_ptr;
    $$ = comp_ptr;
}
| CompoundStmtList ReturnStmt {
    auto ret_stmt = $2;
    comp_stmt_tail -> Next = ret_stmt;
    ref_stmt -> Prev = comp_stmt_tail;
    comp_stmt_tail = comp_stmt_tail -> Next;
    $$ = $1; 
}
| ReturnStmt {
    auto comp_ptr = $1;
    comp_stmt_tail = comp_ptr;
    $$ = comp_ptr;
}
| CompoundStmtList IfStmt {
    auto if_stmt = $2;
    comp_stmt_tail -> Next = if_stmt;
    if_stmt -> Prev = comp_stmt_tail;
    comp_stmt_tail = comp_stmt_tail -> Next;
    $$ = $1; 
}
| IfStmt {
    auto comp_ptr = $1;
    comp_stmt_tail = comp_ptr;
    $$ = comp_ptr;
}
| CompoundStmtList WhileStmt {
    auto while_stmt = $2;
    comp_stmt_tail -> Next = while_stmt;
    while_stmt -> Prev = comp_stmt_tail;
    comp_stmt_tail = comp_stmt_tail -> Next;
    $$ = $1; 
}
| WhileStmt {
    auto comp_ptr = $1;
    comp_stmt_tail = comp_ptr;
    $$ = comp_ptr;
}
| CompoundStmtList ExprStmt SEMICOLON{
    auto expr_stmt = $2;
    comp_stmt_tail -> Next = expr_stmt;
    expr_stmt -> Prev = comp_stmt_tail;
    comp_stmt_tail = comp_stmt_tail -> Next;
    $$ = $1; 
}
| ExprStmt SEMICOLON{
    auto comp_ptr = $1;
    comp_stmt_tail = comp_ptr;
    $$ = comp_ptr;
}
;

ReturnStmt
: RETURN ExprStmt SEMICOLON {
    auto expr_stmt = $2;
    $$ = new ReturnStmt(expr_stmt);
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
    $$ = new IfStmt(expr_stmt, match_stmt, smatch_stmt);
}

UnmatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN Block{
    auto expr_stmt = $3;
    auto match_stmt = $5;
    $$ = new IfStmt(expr_stmt, match_stmt);
}
;

//todo  数组声明action
DeclStmt
: basicType IdentifierList SEMICOLON{
    auto type = $1;
    auto ident_list = $2;
    $$ = new DeclStmt(ident_list);
    $$ -> setType(type);
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET SEMICOLON{
    auto type = $1;
    auto ident = new string($2);
    auto num = $4 -> getVal();
    $$ = ;
}
;

ExprStmt
: {
    $$ = new ExprStmt();
}
| DeclRefStmt ASSIGN ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Assign, ref_stmt, expr_stmt);    
}
| DeclRefStmt OR ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Or, ref_stmt, expr_stmt);  
}  
| DeclRefStmt XOR ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Xor, ref_stmt, expr_stmt);  
}
| DeclRefStmt AND ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::And, ref_stmt, expr_stmt);  
}
| DeclRefStmt PLUS ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Add, ref_stmt, expr_stmt);  
}
| DeclRefStmt MINUS ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Sub, ref_stmt, expr_stmt);  
}
| DeclRefStmt MUL ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Mul, ref_stmt, expr_stmt);  
}
| DeclRefStmt DIV ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Div, ref_stmt, expr_stmt);  
}
| DeclRefStmt EQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Equal, ref_stmt, expr_stmt);  
}
| DeclRefStmt NEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::NotEqual, ref_stmt, expr_stmt);  
}
| DeclRefStmt GRA ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Greater, ref_stmt, expr_stmt);  
}
| DeclRefStmt LES ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Less, ref_stmt, expr_stmt);  
}
| DeclRefStmt GRAEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::GreaterEqual, ref_stmt, expr_stmt);  
}
| DeclRefStmt LESEQ ExprStmt{
    auto ref_stmt = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::LessEqual, ref_stmt, expr_stmt);  
}
| IntegerLiteral OR ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Or, int_li, expr_stmt);  
}  
| IntegerLiteral XOR ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Xor, int_li, expr_stmt);
}
| IntegerLiteral AND ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::And, int_li, expr_stmt);
}
| IntegerLiteral PLUS ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Add, int_li, expr_stmt);
}
| IntegerLiteral MINUS ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Sub, int_li, expr_stmt);
}
| IntegerLiteral MUL ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Mul, int_li, expr_stmt);
}
| IntegerLiteral DIV ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Div, int_li, expr_stmt);
}
| IntegerLiteral EQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Equal, int_li, expr_stmt);
}
| IntegerLiteral NEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::NotEqual, int_li, expr_stmt);
}
| IntegerLiteral GRA ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Greater, int_li, expr_stmt);
}
| IntegerLiteral LES ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::Less, int_li, expr_stmt);
}
| IntegerLiteral GRAEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::GreaterEqual, int_li, expr_stmt);
}
| IntegerLiteral LESEQ ExprStmt{
    auto int_li = $1;
    auto expr_stmt = $3;
    $$ = new BinaryOperatorStmt(BinaryOperatorStmt::LessEqual, int_li, expr_stmt);
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
: IDENTIFIER OPENBRACKET ExprStmt CLOSEBRACKET{
    auto ident = new string($1);
    auto base = new DeclRefStmt(*ident);
    auto expr_stmt = $3;
    $$ = new ArraySubscriptStmt(base, expr_stmt);
}
| IDENTIFIER{
    auto ident = new string($1);
    $$ = new DeclRefStmt(*ident);
}

CallStmt
: IDENTIFIER OPENPAREN ExprStmtList CLOSEPAREN{
    auto ident =new string($1);
    auto expr_list = $3;
    $$ = new CallStmt(*ident, expr_list);
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
: IDENTIFIER{
    auto ident = new string($1);
    auto iden_ptr = new VarDecl(*ident);
    iden_tail = iden_ptr;
    $$ = iden_ptr;
}
| IdentifierList COMMA IDENTIFIER{
    auto i = new string($3);
    auto ident = new VarDecl(*i);
    iden_tail -> Next = ident;
    ident -> Prev = iden_tail;
    iden_tail = iden_tail -> Next;
    $$ = $1;
}
;

%%

void yyerror(CompileUnitDecl& comp_unit, const char *s); {
  cerr << "error: " << s << endl;
};