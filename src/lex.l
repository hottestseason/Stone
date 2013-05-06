%{

#include <stdlib.h>
#include "ast.h"
#include "parse.hh"

%}

DIGIT       [0-9]
INTEGER     {DIGIT}+
DOUBLE      {INTEGER}+\.{INTEGER}

%%

"{" return tLBRACE;
"}" return tRBRACE;
"(" return tLPAREN;
")" return tRPAREN;
"+" return tADD;
"-" return tMINUS;
"*" return tMUL;
"/" return tDIV;
">" return tGT;
"<" return tLT;
"=" return tSET;
"==" return tEQL;
"," return tCOMMA;
";" return tSEMICOLON;
":" return tCOLON;
"\n" return tEOL;

"if" return tIF;
"else" return tELSE;
"def" return tDEF;

{INTEGER} {
    yylval.integer_type = atoi(yytext);
    return tINTEGER;
}

{DOUBLE} {
    yylval.double_type = atof(yytext);
    return tDOUBLE;
}

[A-Za-z][A-Za-z0-9]* {
    yylval.str = new std::string(yytext);
    return tIDENTIFIER;
}

[ \t] ;
%%
