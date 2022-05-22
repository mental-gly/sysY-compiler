%option noyywrap
%option noinput
%option nounput

%{
	#include <cstdlib>
	#include <string>
	#include "sysy.tab.hh"

	using namespace std;
%}

WhiteSpace [ \t\n\r]*
commentBlock_start  "/*"
commentBlock_info  .|\n
commentBlock_end  "*/"
%x COMMENTBLOCK

commentLine_start   "//"
commentLine_info  .
commentLine_end  \n
%x COMMENTLINE

EQ "=="
NEQ "!="
GRAEQ ">="
LESEQ "<="
GRA ">"
LES "<"
ASSIGN "="
PLUSASSIGN "+="
MINUSASSIGN "-="
MULASSIGN "*="
DIVASSIGN "/="
PLUS "+"
MINUS "-"
MUL "*"
DIV "/"
MOD "%"
AND "&&"
OR "||"
BIAND "&"
BIOR "|"
NOT "!"

OPENPAREN "("
CLOSEPAREN ")"
OPENBRACE "{"
CLOSEBRACE "}"
OPENBRACKET "["
CLOSEBRACKET "]"
SEMICOLON ";"
COMMA ","

CONST "const"
VOID "void"
IF "if"
ELSE "else"
WHILE "while"
BREAK "break"
CONTINUE "continue"
RETURN  "return"
INTEGER "int"
BOOLEAN  "bool"
CHAR "char"
STRING "string"

OCT 0[0-7]+
DEC -?[0-9]+
HEX 0[xX][0-9a-fA-F]+
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*

FORMATCHAR [\r\n\t ]+

%%
{FORMATCHAR} {;}
{OPENPAREN} {return OPENPAREN;}
{CLOSEPAREN} {return CLOSEPAREN;}
{OPENBRACKET} {return OPENBRACKET;}
{CLOSEBRACKET} {return CLOSEBRACKET;}
{OPENBRACE} {return OPENBRACE;}
{CLOSEBRACE} {return CLOSEBRACE;}
{SEMICOLON} {return SEMICOLON;}
{COMMA} {return COMMA;}

{ASSIGN} {return ASSIGN;}
{PLUSASSIGN} {return PLUSASSIGN;}
{MINUSASSIGN} {return MINUSASSIGN;}
{MULASSIGN} {return MULASSIGN;}
{DIVASSIGN} {return DIVASSIGN;}
{EQ} {return EQ;}
{NEQ} {return NEQ;}
{GRA} {return GRA;}
{LES} {return LES;}
{GRAEQ} {return GRAEQ;}
{LESEQ} {return LESEQ;}
{PLUS} {return PLUS;}
{MINUS} {return MINUS;}
{MUL} {return MUL;}
{DIV} {return DIV;}
{MOD} {return MOD;}
{AND} {return AND;}
{OR} {return OR;}
{BIAND} {return BIAND;}
{BIOR} {return BIOR;}
{NOT} {return NOT;}

{CONST} {return CONST;}
{VOID} {return VOID;}
{IF} {return IF;}
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{BREAK} {return BREAK;}
{CONTINUE} {return CONTINUE;}
{RETURN} {return RETURN;}

{INTEGER} {return INTEGER;}
{BOOLEAN} {return BOOLEAN;}
{CHAR} {return CHAR;}

{OCT} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{DEC} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{HEX} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{IDENTIFIER} {yylval.str_val = new string(yytext); return IDENTIFIER;}

{WhiteSpace} {}
{commentBlock_start} {BEGIN COMMENTBLOCK;}
<COMMENTBLOCK>{commentBlock_info} {}
<COMMENTBLOCK>{commentBlock_end} {BEGIN INITIAL;}
{commentLine_start} {BEGIN COMMENTLINE;}
<COMMENTLINE>{commentLine_info} {}
<COMMENTLINE>{commentLine_end} {BEGIN INITIAL;}

. {return yytext[0];}

%%