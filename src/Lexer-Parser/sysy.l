%option noyywrap

%{
	#include <cstdlib>
	#include <string>
	#include <iostream>
	#include <map>
	#include "parser.tab.hh"

    extern bool ImportStdio;
    extern bool ImportString;
    bool ImportStdio = false;
    bool ImportString = false;
	using namespace std;
	typedef std::map<std::string, std::string> TStrStrMap;
	typedef std::pair<std::string, std::string> TStrStrPair;
	TStrStrMap tMap;
	string temp;
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

defineLine_start   "#define"
defineLine_id  [a-zA-Z_][a-zA-Z0-9_]*
defineLine_val [^ ]*
defineLine_end  \n
%x DEFINELINE

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

TRUE "true"
FALSE "false"
CONST "const"
VOID "void"
IF "if"
ELSE "else"
WHILE "while"
BREAK "break"
CONTINUE "continue"
RETURN  "return"
INTEGER "int"
DOUBLE "double"
BOOLEAN  "bool"
CHAR "char"
STRING "string"
STDIO "use std.io;"
STDSTRING "use std.string;"
CAST "cast"
GENERIC_BEGIN "::<"
GENERIC_END ">::"
escape \\n|\\t|\\r|\\\'|\\\"|\\0
s_char [^\\\"\n]|{escape}
c_char [^\\\'\n]|{escape}
StringLiteral \"{s_char}+\"
CharConst \'{c_char}+\'

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
{STDIO} {
    ImportStdio = true;
}
{STDSTRING} {
    ImportString = true;
}
{StringLiteral} {
    // Remove " "
    yylval.str_val = new std::string(yytext + 1, yyleng - 2);
    return CONSTSTRING;
}

{INTEGER} {return INTEGER;}
{BOOLEAN} {return BOOLEAN;}
{DOUBLE} { return DOUBLE; }
{CHAR} {return CHAR;}
{CharConst} {
    // translation.
    if (yytext[1] == '\\') {
        if (yytext[2] == 'n') {
            yylval.int_val = (int)'\n';
        }
        if (yytext[2] == 't') {
            yylval.int_val = (int)'\t';
        }
        if (yytext[2] == '\\') {
            yylval.int_val = (int)'\\';
        }
        if (yytext[2] == '0') {
            yylval.int_val = (int)0;
        }
    }
    else {
        yylval.int_val = (int)yytext[1];
    }
    return CharConst;
 }

 {CAST} { return CAST; }
 {GENERIC_BEGIN} { return GENERIC_BEGIN; }
 {GENERIC_END} { return GENERIC_END; }

{TRUE} { return TRUE; }
{FALSE} { return FALSE; }
{OCT} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{DEC} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{HEX} {yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST;}
{IDENTIFIER} {
		yylval.str_val = new std::string(yytext);
		return IDENTIFIER;
}

{WhiteSpace} {}
{commentBlock_start} {BEGIN COMMENTBLOCK;}
<COMMENTBLOCK>{commentBlock_info} {}
<COMMENTBLOCK>{commentBlock_end} {BEGIN INITIAL;}
{commentLine_start} {BEGIN COMMENTLINE;}
<COMMENTLINE>{commentLine_info} {}
<COMMENTLINE>{commentLine_end} {BEGIN INITIAL;}



. {return yytext[0];}

%%