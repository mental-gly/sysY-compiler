/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SYSY_TAB_HPP_INCLUDED
# define YY_YY_SYSY_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "parser.y" /* yacc.c:1909  */

#include <memory>
#include <string>
#include "AST/Decl.h"
#include "AST/Stmt.h"
#include "AST/TypeInfo.h"

#line 52 "parser.tab.hh" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_CHAR = 258,
    T_INT = 259,
    T_STRING = 260,
    T_BOOL = 261,
    T_VOID = 262,
    COMMA = 263,
    SEMICOLON = 264,
    OPENPAREN = 265,
    CLOSEPAREN = 266,
    OPENBRACE = 267,
    CLOSEBRACE = 268,
    OPENBRACKET = 269,
    CLOSEBRACKET = 270,
    ADDR = 271,
    MOD = 272,
    BIAND = 273,
    BIOR = 274,
    NOT = 275,
    VOID = 276,
    ASSIGN = 277,
    PLUSASSIGN = 278,
    MINUSASSIGN = 279,
    MULASSIGN = 280,
    DIVASSIGN = 281,
    CONST = 282,
    IF = 283,
    ELSE = 284,
    WHILE = 285,
    BREAK = 286,
    CONTINUE = 287,
    RETURN = 288,
    EQ = 289,
    GRAEQ = 290,
    LESEQ = 291,
    NEQ = 292,
    GRA = 293,
    LES = 294,
    CHAR = 295,
    INTEGER = 296,
    STRING = 297,
    BOOLEAN = 298,
    INT_CONST = 299,
    IDENTIFIER = 300,
    OR = 301,
    XOR = 302,
    AND = 303,
    PLUS = 304,
    MINUS = 305,
    MUL = 306,
    DIV = 307
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 50 "parser.y" /* yacc.c:1909  */

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

#line 131 "parser.tab.hh" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (CompileUnitDecl& comp_unit);

#endif /* !YY_YY_SYSY_TAB_HPP_INCLUDED  */
