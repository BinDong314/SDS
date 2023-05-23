%{
 
/*
 * Parser.y file
 * To generate the parser run: "bison Parser.y"
 */
 
#include "sds-condition-tree.h"
#include "sds-cond-parser.h"
#include "sds-cond-lexer.h"
 
int yyerror(SDS_Condition_tree **expression, yyscan_t scanner, const char *msg) {
    // Add error handling routine as needed
  printf("Something is wrong %s \n", msg);
  return 0;
}
 
%}
 
%code requires{
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif
}
 
%output  "sds-cond-parser.c"
%defines "sds-cond-parser.h"
 
%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param {SDS_Condition_tree ***expression} 
%parse-param { yyscan_t scanner }
 
%union {
  double               dv;
  char                *varname;
  int                  op_type;
  SDS_Condition_tree  *expression;
}
 
%token             END	     0   "end of input"
%token <dv>        NUMBER 	 "number"
%token <op_type>   LOGICOP       "logical operator"
%token <op_type>   RANGEOP       "range operator"
%token <varname>   NAMESTR       "variable name"

%type <expression> queries 
%type <expression> queryExpr


%% /* Grammar rules */

input
    : queries { **expression = $1; }
    ;


queries:
queryExpr[l] LOGICOP queries[r] { $$=create_nonleaf($2, $l, $r);}
|
queryExpr{
  $$=$1;
}
;

queryExpr:
NUMBER[lv] RANGEOP[lt] NAMESTR RANGEOP[rt] NUMBER[rv]{
  $$=create_leaf_between($3, $lv, $lt,  $rv, $rt);
}
|
NAMESTR[n] RANGEOP[t] NUMBER[v] {
  $$=create_leaf_oneside($n, $v, $t);
  //printf("Find a one side expr !\n");
};

%%
