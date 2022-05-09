%{
    #include<math.h>

	int id_count = 0;
	char id_list[100][20];
%}

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
GE ">="
LE "<="
GT ">"
LT "<"
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

CONST "const"
VOID "void"
IF "if"
ELSE "else"
WHILE "while"
BREAK "break"
CONTINUE "continue"
RETURN  "return"
INT "int"
BOOLEAN  "bool"
CHAR "char"

OCT 0[0-7]+
DEC -?[0-9]+
HEX 0[xX][0-9a-fA-F]+
IDENTIFIER [a-zA-Z_][a-zA-Z0-9_]*

FORMATCHAR [\r\n\t ]+

%%
{FORMATCHAR} {;}
{OPENPAREN} {fprintf(yyout, "%12s%8s\n", "OPENPAREN",yytext);}
{CLOSEPAREN} {fprintf(yyout, "%12s%8s\n", "CLOSEPAREN", yytext);}
{OPENBRACE} {fprintf(yyout, "%12s%8s\n", "OPENBRACE", yytext);}
{CLOSEBRACE} {fprintf(yyout, "%12s%8s\n", "CLOSEBRACE", yytext);}
{SEMICOLON} {fprintf(yyout, "%12s%8s\n", "SEMICOLON", yytext);}

{ASSIGN} {fprintf(yyout, "%12s%8s\n", "ASSIGN", yytext);}
{PLUSASSIGN} {fprintf(yyout, "%12s%8s\n", "PLUSASSIGN", yytext);}
{MINUSASSIGN} {fprintf(yyout, "%12s%8s\n", "MINUSASSIGN", yytext);}
{MULASSIGN} {fprintf(yyout, "%12s%8s\n", "MULASSIGN", yytext);}
{DIVASSIGN} {fprintf(yyout, "%12s%8s\n", "DIVASSIGN", yytext);}
{EQ} {fprintf(yyout, "%12s%8s\n", "EQ", yytext);}
{NEQ} {fprintf(yyout, "%12s%8s\n", "NEQ", yytext);}
{GT} {fprintf(yyout, "%12s%8s\n", "GT", yytext);}
{LT} {fprintf(yyout, "%12s%8s\n", "LT", yytext);}
{GE} {fprintf(yyout, "%12s%8s\n", "GE", yytext);}
{LE} {fprintf(yyout, "%12s%8s\n", "LE", yytext);}
{PLUS} {fprintf(yyout, "%12s%8s\n", "PLUS", yytext);}
{MINUS} {fprintf(yyout, "%12s%8s\n", "MINUS", yytext);}
{MUL} {fprintf(yyout, "%12s%8s\n", "MUL", yytext);}
{DIV} {fprintf(yyout, "%12s%8s\n", "DIV", yytext);}
{MOD} {fprintf(yyout, "%12s%8s\n", "MOD", yytext);}
{AND} {fprintf(yyout, "%12s%8s\n", "AND", yytext);}
{OR} {fprintf(yyout, "%12s%8s\n", "OR", yytext);}
{BIAND} {fprintf(yyout, "%12s%8s\n", "BIAND", yytext);}
{BIOR} {fprintf(yyout, "%12s%8s\n", "BIOR", yytext);}
{NOT} {fprintf(yyout, "%12s%8s\n", "NOT", yytext);}

{CONST} {fprintf(yyout, "%12s%8s\n", "CONST", yytext);}
{VOID} {fprintf(yyout, "%12s%8s\n", "VOID", yytext);}
{IF} {fprintf(yyout, "%12s%8s\n", "IF", yytext);}
{ELSE} {fprintf(yyout, "%12s%8s\n", "ELSE", yytext);}
{WHILE} {fprintf(yyout, "%12s%8s\n", "WHILE", yytext);}
{BREAK} {fprintf(yyout, "%12s%8s\n", "BREAK", yytext);}
{CONTINUE} {fprintf(yyout, "%12s%8s\n", "CONTINUE", yytext);}
{RETURN} {fprintf(yyout, "%12s%8s\n", "RETURN", yytext);}

{INT} {fprintf(yyout, "%12s%8s\n", "INT", yytext);}
{BOOLEAN} {fprintf(yyout, "%12s%8s\n", "BOOLEAN", yytext);}
{CHAR} {fprintf(yyout, "%12s%8s\n", "CHAR", yytext);}

{OCT} {fprintf(yyout, "%12s%8s%8s\n", "OCT", yytext, yytext);}
{DEC} {fprintf(yyout, "%12s%8s%8s\n", "DEC", yytext, yytext);}
{HEX} {fprintf(yyout, "%12s%8s%8s\n", "HEX", yytext, yytext);}
{IDENTIFIER} {
	int flag = 0;
	int id = 0;
	for (int i = 0; i < id_count; i++) {
		if (strcmp(yytext, id_list[i])==0) {
			flag = 1;
			id = i;
			break;
		}
	}
	if (flag == 0) {
		id = id_count;
		strcpy(id_list[id], yytext);
		id_count++;
	}
	fprintf(yyout, "%12s%8s%8d\n", "IDENTIFIER", yytext, id);
}

{commentBlock_start} {BEGIN COMMENTBLOCK;}
<COMMENTBLOCK>{commentBlock_info} {}
<COMMENTBLOCK>{commentBlock_end} {BEGIN INITIAL;}
{commentLine_start} {BEGIN COMMENTLINE;}
<COMMENTLINE>{commentLine_info} {}
<COMMENTLINE>{commentLine_end} {BEGIN INITIAL;}

%%

int main(int argc, char **argv)
{
  yylex();
  yywrap();
}
int yywrap()
{
  return 0;
}
