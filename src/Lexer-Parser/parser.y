%{
    int yylex();
    int yyerror();
    #include "decl.h"
%}

%union {
  std::string *str_val;
  int int_val;
}

%parse-param { std::unique_ptr<Stmt> &stmt }
%type <stmt> DeclStmt CompoundStmt ExprStmt CallStmt DeclRef ArraySubscript BinaryOperator IntegerLiteral FloatingLiteral
%token T_CHAR T_INT T_STRING T_BOOL T_VOID
%token COMMA SEMICOLON OPENPAREN CLOSEPAREN OPENBRACE CLOSEBRACE OPENBRACKET CLOSEBRACKET
%token ADDR
%token ASSIGN PLUSASSIGN MINUSASSIGN MULASSIGN DIVASSIGN
%token CONST IF_ ELSE WHILE_ FOR_ BREAK CONTINUE RETURN
%token EQ GRAEQ LESEQ NEQ GRA LES
%token IDENTIFIER CHAR INTEGER STRING BOOL
%token IF ELSE

%left EQ
%left OR XOR AND
%left PLUS MINUS
%left MUL DIV

%%

CompileUnit
: FunctionDecl{
    auto comp_unit = new CompileUnit();
    $$ = comp_unit;
}
;

FunctionDecl 
: basicType IDENTIFIER OPENPAREN Paramlist CLOSEPAREN OPENBRACE CompoundStmt CLOSEBRACE{
    TypeInfo *type = TypeContext::find($1);
    auto func_decl = new FunctionDecl(type, $4);
    $$ = func_decl;
} 
| basicType IDENTIFIER OPENBRACE Paramlist CLOSEBRACE SEMICOLON{
    TypeInfo *type = TypeContext::find($1);
    auto func_decl = new FunctionDecl(type, $4);
    $$ = func_decl; 
}
;

ParamList
: ParamDecl{
    auto param_list = new ParamList();
    param_list -> ?($1);//link funtion
    $$ = param_list; 
}
| ParamList COMMA ParamDecl{
    $$ = $1;
    $$ -> ?($3);
}
;

ParamDecl
: basicType IDENTIFIER{
    ? //type how to transport
    VarDecl *var = new VarDecl(Decl::kVarDecl, $2);
    auto para_decl = new ParamDecl(var);
    $$ = para_decl;
}
;

CompoundStmt
: CompoundStmt DeclStmt {
    $$ = $1;
    $$ -> CreateSubStmt($2);
}
| CompoundStmt ExprStmt SEMICOLON{
    $$ = $1;
    $$ -> CreateSubStmt($2);
}
| CompoundStmt IfStmt{
    $$ = $1;
    $$ -> CreateSubStmt($2);
}
| DeclStmt {
    auto comp_stmt = new CompoundStmt();
    comp_stmt -> CreateSubStmt($1);
    $$ = comp_stmt;
}
| ExprStmt SEMICOLON {
    auto comp_stmt = new CompoundStmt();
    comp_stmt -> CreateSubStmt($1);
    $$ = comp_stmt;
}
| ReturnStmt {
    auto comp_stmt = new CompoundStmt();
    comp_stmt -> CreateSubStmt($1);
    $$ = comp_stmt;
}
| IfStmt{
    auto comp_stmt = new CompoundStmt();
    comp_stmt -> CreateSubStmt($1);
    $$ = comp_stmt;
}
;

IfStmt
: MatchedStmt{
    
}
| UnmatchedStmt{

}
;

MatchedStmt
: IF OPENPAREN ExprStmtList CLOSEPAREN MatchedStmt ELSE MatchedStmt{

}
| OPENBRACE CompoundStmt CLOSEBRACE{

}
;

UnmatchedStmt
: IF OPENPAREN ExprStmtList CLOSEPAREN IfStmt{

}
| IF OPENPAREN ExprStmtList CLOSEPAREN MatchedStmt ELSE UnmatchedStmt{

}
;

DeclStmt
: basicType IdentifierList SEMICOLON{
    auto decl_stmt = new DeclStmt($2);
    TypeInfo *type = TypeContext::find($1);
    decl_stmt -> setType(type);
    $$ = decl_stmt;
}
| basicType IDENTIFIER OPENBRACKET IntegerLiteral CLOSEBRACKET SEMICOLON{
    TypeInfo *type = REGISTER_ARRAY($1, $4->getVal());
    VarDecl *var = new VarDecl(Decl::kVarDecl, $2);
    auto decl_stmt = new DeclStmt(var);
    decl_stmt -> setType(type);
    $$ = decl_stmt;
}
;

LiteralList
: IntegerLiteral{
    auto lit_list = new LiteralList();
    lit_list -> ?() = $1; //link
    $$ = lit_list;
}
| LiteralList COMMA IntegerLiteral{
    $$ = $1;
    $$ -> ?() = $3;
}
| FloatingLiteral{
    auto lit_list = new LiteralList();
    lit_list -> ?() = $1;
    $$ = lit_list;
}
| LiteralList COMMA FloatingLiteral{
    $$ = $1;
    $$ -> ?() = $3;
}
;

ExprStmt
: DeclRefStmt ASSIGN ExprStmt {
    auto _assign = new BinaryOperator(BinaryOperator::Assign, $1, $3);
    $$ = _assign;
}
| DeclRefStmt OR ExprStmt{
    auto _or = new BinaryOperator(BinaryOperator::Or, $1, $3);
    $$ = _or;
}  
| DeclRefStmt XOR ExprStmt{
    auto _xor = new BinaryOperator(BinaryOperator::Xor, $1, $3);
    $$ = _xor;
}
| DeclRefStmt AND ExprStmt{
    auto _and = new BinaryOperator(BinaryOperator::And, $1, $3);
    $$ = _and;
}
| DeclRefStmt PLUS ExprStmt{
    auto _plus = new BinaryOperator(BinaryOperator::Add, $1, $3);
    $$ = _plus;
}
| DeclRefStmt MINUS ExprStmt{
    auto _minus = new BinaryOperator(BinaryOperator::Sub, $1, $3);
    $$ = _minus;
}
| DeclRefStmt MUL ExprStmt{
    auto _mul = new BinaryOperator(BinaryOperator::Mul, $1, $3);
    $$ = _mul;
}
| DeclRefStmt DIV ExprStmt{
    auto _div = new BinaryOperator(BinaryOperator::UDiv, $1, $3);
    $$ = _div;
}
| IntegerLiteral OR ExprStmt{
    auto _or = new BinaryOperator(BinaryOperator::Or, $1, $3);
    $$ = _or;
}  
| IntegerLiteral XOR ExprStmt{
    auto _xor = new BinaryOperator(BinaryOperator::Xor, $1, $3);
    $$ = _xor;
}
| IntegerLiteral AND ExprStmt{
    auto _and = new BinaryOperator(BinaryOperator::And, $1, $3);
    $$ = _and;
}
| IntegerLiteral PLUS ExprStmt{
    auto _plus = new BinaryOperator(BinaryOperator::Add, $1, $3);
    $$ = _plus;
}
| IntegerLiteral MINUS ExprStmt{
    auto _minus = new BinaryOperator(BinaryOperator::Sub, $1, $3);
    $$ = _minus;
}
| IntegerLiteral MUL ExprStmt{
    auto _mul = new BinaryOperator(BinaryOperator::Mul, $1, $3);
    $$ = _mul;
}
| IntegerLiteral DIV ExprStmt{
    auto _div = new BinaryOperator(BinaryOperator::UDiv, $1, $3);
    $$ = _div;
}
| FloatingLiteral OR ExprStmt{
    auto _or = new BinaryOperator(BinaryOperator::Or, $1, $3);
    $$ = _or;
}  
| FloatingLiteral XOR ExprStmt{
    auto _xor = new BinaryOperator(BinaryOperator::Xor, $1, $3);
    $$ = _xor;
}
| FloatingLiteral AND ExprStmt{
    auto _and = new BinaryOperator(BinaryOperator::And, $1, $3);
    $$ = _and;
}
| FloatingLiteral PLUS ExprStmt{
    auto _plus = new BinaryOperator(BinaryOperator::Add, $1, $3);
    $$ = _plus;
}
| FloatingLiteral MINUS ExprStmt{
    auto _minus = new BinaryOperator(BinaryOperator::Sub, $1, $3);
    $$ = _minus;
}
| FloatingLiteral MUL ExprStmt{
    auto _mul = new BinaryOperator(BinaryOperator::Mul, $1, $3);
    $$ = _mul;
}
| FloatingLiteral DIV ExprStmt{
    auto _div = new BinaryOperator(BinaryOperator::UDiv, $1, $3);
    $$ = _div;
}
| DeclRefStmt {
    auto expr_stmt = new ExprStmt();
    $$ -> ?() = $1; //要把exprstmt和declrestmt找关系连起来,下面三个同理
    $$ = expr_stmt;
}
| IntegerLiteral {
    auto expr_stmt = new ExprStmt();
    $$ -> ?() = $1;
    $$ = expr_stmt;
}
| FloatingLiteral {
    auto expr_stmt = new ExprStmt();
    $$ -> ?() = $1;
    $$ = expr_stmt;
}
| CallStmt {
    auto expr_stmt = new ExprStmt();
    $$ -> ?() = $1;
    $$ = expr_stmt;
}
;

CallStmt
: IDENTIFIER OPENPAREN ?ExprStmtList CLOSEPAREN SEMICOLON {
    auto call_stmt = new CallStmt($1, $3);
    $$ = call_stmt;
}
;

ExprStmtList
: ExprStmt {
    auto expr_stmtlist = new ExprStmtList($1);//? gouzaohanshu 
    $$ = expr_stmtlist;
}
| ExprStmtList COMMA ExprStmt{
    $$ = $1;
    $$ -> ?() = $3; //link
}
;

DeclRefStmt
: IDENTIFIER OPENBRACKET ExprStmt CLOSEBRACKET{
    ?auto array_sub = new ArraySubscriptStmt($1, $3); // gouzaohanshu 
    $$ = array_sub;
}
| IDENTIFIER{

}

IdentifierList
: IDENTIFIER{
    auto identifier_list = new IdentifierList($1);//? 构造函数没有
    $$ = identifier_list;
}
| IdentifierList COMMA IDENTIFIER{
    $$ = $1;
    $$ -> ?() = $3; //连接的函数没有
}
;

ReturnStmt
: RETURN ExprStmt SEMICOLON {
    auto return_stmt = new ReturnStmt($2);
    $$ = return_stmt;
}
| RETURN SEMICOLON {
    auto return_stmt = new ReturnStmt(nullptr);
    $$ = return_stmt;
}
;

basicType
: T_INT {
    $$->type = TYPE_INT;
}
| T_CHAR {
    $$->type = TYPE_CHAR;
}
| T_BOOL {
    $$->type = TYPE_BOOL;
}
| T_VOID {
    $$->type = TYPE_VOID;
}
;