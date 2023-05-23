%{
 
/*
 * Lexer.l file
 * To generate the lexical analyzer run: "flex Lexer.l"
 */
 
#include "sds-condition-tree.h"
#include "sds-cond-parser.h"

#include <stdio.h>
 
%}
 
%option outfile="sds-cond-lexer.c" header-file="sds-cond-lexer.h"
%option warn nodefault
 
%option reentrant noyywrap never-interactive nounistd
%option bison-bridge
 
NUMBER  [-+]?([0-9]+[.]?|[0-9]*[.][0-9]+)([eE][-+]?[0-9]+)?
VARNAME	[a-zA-Z][a-zA-Z0-9_\/]*
LSEP    [ \t\v\n\v,;]
WS	[ \n\t\r\v]
 
 
%%
[-+]?[0-9]*[.]?[0-9]+([eE][-+]?[0-9]+)?	   {yylval->dv = atof(yytext); return NUMBER;}
"||"|[oO][rR]	                           {yylval->op_type = SDS_OR;  return LOGICOP;}
"&&"|[aA][nN][dD]	                   {yylval->op_type = SDS_AND; return LOGICOP;}
"!="			                   {yylval->op_type = SDS_NE;  return RANGEOP;}
"=="			                   {yylval->op_type = SDS_EQ;  return RANGEOP;}
"<"			                   {yylval->op_type = SDS_LT;  return RANGEOP;}
">"			                   {yylval->op_type = SDS_GT;  return RANGEOP;}
"<="			                   {yylval->op_type = SDS_LE;  return RANGEOP;}
">="			                   {yylval->op_type = SDS_GT;  return RANGEOP;}
[a-zA-Z][a-zA-Z0-9_\/]*        		   {yylval->varname = strdup(yytext);  return NAMESTR;}
[ \n\t\r\v]	                           {}
.					   {}
%%
 
