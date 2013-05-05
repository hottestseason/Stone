%{

#include <stdlib.h>
#include "ast.h"
#include "parse.hh"

%}

DIGIT       [0-9]
INTEGER     {DIGIT}+

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
"\n" return tEOL;

"if" return tIF;
"else" return tELSE;
"def" return tDEF;

{INTEGER} {
    yylval.number = atof(yytext);
    return tNUMBER;
}

[A-Za-z][A-Za-z0-9]* {
    yylval.str = new string(yytext);
    return tIDENTIFIER;
}

[ \t] ;
%%
