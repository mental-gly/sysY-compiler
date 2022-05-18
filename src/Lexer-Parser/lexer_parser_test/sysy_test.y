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

int yylex();
void yyerror(std::unique_ptr<std::string> &ast, const char *s);
extern FILE* yyin;

using namespace std;

%}

%parse-param { std::unique_ptr<std::string> &ast }
%union {
  std::string *str_val;
  int int_val;
}

%type <str_val> FunctionDecl basicType CompoundStmt ParamList ParamDecl DeclStmt IdentifierList ReturnStmt ExprStmt DeclRefStmt IntegerLiteral CallStmt ExprStmtList CompileUnit Program IfStmt MatchedStmt UnmatchedStmt WhileStmt
%token T_CHAR T_INT T_STRING T_BOOL T_VOID
%token COMMA SEMICOLON OPENPAREN CLOSEPAREN OPENBRACE CLOSEBRACE OPENBRACKET CLOSEBRACKET
%token ADDR
%token MOD BIAND BIOR NOT VOID
%token ASSIGN PLUSASSIGN MINUSASSIGN MULASSIGN DIVASSIGN
%token CONST IF ELSE WHILE FOR_ BREAK CONTINUE RETURN
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
    ast = unique_ptr<string>($1);
}

CompileUnit
: CompileUnit FunctionDecl{
    auto unit = unique_ptr<string>($1);
    auto func_decl = unique_ptr<string>($2);
    $$ = new string(*unit + *func_decl);
}
| FunctionDecl{
    auto func_decl = unique_ptr<string>($1);
    $$ = new string(*func_decl);
}
;

FunctionDecl 
: basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN OPENBRACE CompoundStmt CLOSEBRACE{
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    auto param_list = unique_ptr<string>($4);
    auto block = unique_ptr<string>($7);
    $$ = new string(*type + " " + *ident + "(" + *param_list + ") " + "{ " + *block + "}");
} 
| basicType IDENTIFIER OPENPAREN ParamList CLOSEPAREN SEMICOLON{
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    auto param_list = unique_ptr<string>($4);
    $$ = new string(*type + " " + *ident + "("+ *param_list+")" + ";");
}
;

ParamList
: {
    $$ = new string("");  
}
| ParamDecl{
    auto param_decl = unique_ptr<string>($1);
    $$ = new string(*param_decl);  
}
| ParamList COMMA ParamDecl{
    auto param_list = unique_ptr<string>($1);
    auto param_decl = unique_ptr<string>($3);
    $$ = new string(*param_list + "," + *param_decl);  
}
;

ParamDecl
: basicType IDENTIFIER{
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    $$ = new string(*type +" "+ *ident);
}
; 

basicType
: INTEGER {
    $$ = new string("int");
}
| CHAR {
    $$ = new string("char");
}
| BOOLEAN {
    $$ = new string("bool");
}
| VOID {
    $$ = new string("void");
}
;

CompoundStmt
: CompoundStmt DeclStmt {
    auto comp_stmt = unique_ptr<string>($1);
    auto decl_stmt = unique_ptr<string>($2);
    $$ = new string(*comp_stmt + *decl_stmt);  
}
| DeclStmt {
    auto decl_stmt = unique_ptr<string>($1);
    $$ = new string(*decl_stmt);
}
| CompoundStmt ReturnStmt {
    auto comp_stmt = unique_ptr<string>($1);
    auto ret_stmt = unique_ptr<string>($2);
    $$ = new string(*comp_stmt + *ret_stmt);  
}
| ReturnStmt {
    auto ret_stmt = unique_ptr<string>($1);
    $$ = new string(*ret_stmt);
}
| CompoundStmt IfStmt {
    auto comp_stmt = unique_ptr<string>($1);
    auto if_stmt = unique_ptr<string>($2);
    $$ = new string(*comp_stmt + *if_stmt);  
}
| IfStmt {
    auto if_stmt = unique_ptr<string>($1);
    $$ = new string(*if_stmt);
}
| CompoundStmt WhileStmt {
    auto comp_stmt = unique_ptr<string>($1);
    auto while_stmt = unique_ptr<string>($2);
    $$ = new string(*comp_stmt + *while_stmt);  
}
| WhileStmt {
    auto while_stmt = unique_ptr<string>($1);
    $$ = new string(*while_stmt);
}
| CompoundStmt ExprStmt SEMICOLON{
    auto comp_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($2);
    $$ = new string(*comp_stmt + *expr_stmt + ";");  
}
| ExprStmt SEMICOLON{
    auto expr_stmt = unique_ptr<string>($1);
    $$ = new string(*expr_stmt + ";");
}
;

ReturnStmt
: RETURN SEMICOLON {
    $$ = new string("return;");
}
| RETURN ExprStmt SEMICOLON {
    auto expr_stmt = unique_ptr<string>($2);
    $$ = new string("return " + *expr_stmt + ";");
}
;

IfStmt
: MatchedStmt{
    auto match_stmt = unique_ptr<string>($1);
    $$ = new string(*match_stmt);
}
| UnmatchedStmt{
    auto unmatch_stmt = unique_ptr<string>($1);
    $$ = new string(*unmatch_stmt);
}
;

WhileStmt
: WHILE OPENPAREN ExprStmt CLOSEPAREN CompoundStmt{
    auto expr_stmt = unique_ptr<string>($3);
    auto comp_stmt = unique_ptr<string>($5);
    $$ = new string("while(" + *expr_stmt + ")" + *comp_stmt);
}

MatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN MatchedStmt ELSE MatchedStmt{
    auto expr_stmt = unique_ptr<string>($3);
    auto match_stmt = unique_ptr<string>($5);
    auto smatch_stmt = unique_ptr<string>($7);
    $$ = new string("if(" + *expr_stmt + ")" + *match_stmt + "else" + *smatch_stmt);
}
| OPENBRACE CompoundStmt CLOSEBRACE{
    auto comp_stmt = unique_ptr<string>($2);
    $$ = new string("{" + *comp_stmt + "}");
}
;

UnmatchedStmt
: IF OPENPAREN ExprStmt CLOSEPAREN IfStmt{
    auto expr_stmt = unique_ptr<string>($3);
    auto if_stmt = unique_ptr<string>($5);
    $$ = new string("if(" + *expr_stmt + ")" + *if_stmt);
}
| IF OPENPAREN ExprStmt CLOSEPAREN MatchedStmt ELSE UnmatchedStmt{
    auto expr_stmt = unique_ptr<string>($3);
    auto match_stmt = unique_ptr<string>($5);
    auto unmatch_stmt = unique_ptr<string>($7);
    $$ = new string("if(" + *expr_stmt + ")" + *match_stmt + "else" + *unmatch_stmt);
}
;

DeclStmt
: basicType IdentifierList SEMICOLON{
    auto type = unique_ptr<string>($1);
    auto ident_list = unique_ptr<string>($2);
    $$ = new string(*type + " "+ *ident_list + ";");
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET SEMICOLON{
    auto type = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($2);
    auto num = unique_ptr<string>($4);
    $$ = new string(*type + " "+ *ident + "[" + *num +"]" + ";");
}
;

ExprStmt
: DeclRefStmt ASSIGN ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "=" + *expr_stmt);    
}
| DeclRefStmt OR ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "||" + *expr_stmt);
}  
| DeclRefStmt XOR ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "|" + *expr_stmt);
}
| DeclRefStmt AND ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "&&" + *expr_stmt);
}
| DeclRefStmt PLUS ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "+" + *expr_stmt);
}
| DeclRefStmt MINUS ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "-" + *expr_stmt);
}
| DeclRefStmt MUL ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "*" + *expr_stmt);
}
| DeclRefStmt DIV ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "/" + *expr_stmt);
}
| DeclRefStmt EQ ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "==" + *expr_stmt);
}
| DeclRefStmt NEQ ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "!=" + *expr_stmt);
}
| DeclRefStmt GRA ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + ">" + *expr_stmt);
}
| DeclRefStmt LES ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "<" + *expr_stmt);
}
| DeclRefStmt GRAEQ ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + ">=" + *expr_stmt);
}
| DeclRefStmt LESEQ ExprStmt{
    auto ref_stmt = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ref_stmt + "<=" + *expr_stmt);
}
| IntegerLiteral OR ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "||" + *expr_stmt);
}  
| IntegerLiteral XOR ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "|" + *expr_stmt);
}
| IntegerLiteral AND ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "&&" + *expr_stmt);
}
| IntegerLiteral PLUS ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "+" + *expr_stmt);
}
| IntegerLiteral MINUS ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "-" + *expr_stmt);
}
| IntegerLiteral MUL ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "*" + *expr_stmt);
}
| IntegerLiteral DIV ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "/" + *expr_stmt);
}
| IntegerLiteral EQ ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "==" + *expr_stmt);
}
| IntegerLiteral NEQ ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "!=" + *expr_stmt);
}
| IntegerLiteral GRA ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + ">" + *expr_stmt);
}
| IntegerLiteral LES ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "<" + *expr_stmt);
}
| IntegerLiteral GRAEQ ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + ">=" + *expr_stmt);
}
| IntegerLiteral LESEQ ExprStmt{
    auto int_li = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*int_li + "<=" + *expr_stmt);
}
| DeclRefStmt {
    auto ref_stmt = unique_ptr<string>($1);
    $$ = new string(*ref_stmt);
}
| IntegerLiteral {
    auto int_li = unique_ptr<string>($1);
    $$ = new string(*int_li);
}
| CallStmt {
    auto call_stmt = unique_ptr<string>($1);
    $$ = new string(*call_stmt);
}
;

IntegerLiteral
: INT_CONST {
    $$ = new string(to_string($1));
}
;

DeclRefStmt
: IDENTIFIER OPENBRACKET ExprStmt CLOSEBRACKET{
    auto ident = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*ident + "["+ *expr_stmt +"]");
}
| IDENTIFIER{
    auto ident = unique_ptr<string>($1);
    $$ = new string(*ident);
}

CallStmt
: IDENTIFIER OPENPAREN ExprStmtList CLOSEPAREN{
    auto call_stmt = unique_ptr<string>($1);
    auto expr_list = unique_ptr<string>($3);
    $$ = new string(*call_stmt + "("+ *expr_list + ")");
}
;

ExprStmtList
: ExprStmt {
    auto expr_stmt = unique_ptr<string>($1);
    $$ = new string(*expr_stmt);
}
| ExprStmtList COMMA ExprStmt{
    auto expr_list = unique_ptr<string>($1);
    auto expr_stmt = unique_ptr<string>($3);
    $$ = new string(*expr_list + ","+ *expr_stmt);
}
;

IdentifierList
: IDENTIFIER{
    auto ident = unique_ptr<string>($1);
    $$ = new string(*ident);
}
| IdentifierList COMMA IDENTIFIER{
    auto ident_list = unique_ptr<string>($1);
    auto ident = unique_ptr<string>($3);
    $$ = new string(*ident_list +"," + *ident);
}
;

%%

void yyerror(unique_ptr<string> &ast, const char *s) {
  cerr << "error: " << s << endl;
};

int main() {
  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen("test.txt", "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<string> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // 输出解析得到的 AST, 其实就是个字符串
  cout << *ast << endl;
  return 0;
}
