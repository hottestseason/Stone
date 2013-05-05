%{

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "ast.h"

using namespace std;

void yyerror(const char *msg) {
    fprintf(stderr, "parser error near %s\n", msg);
}

extern "C" {
    int yyparse();
}
extern int yylex();

TopAST *ast;

%}

%union {
    double number;
    AST *ast;
    string *str;
}

%token<number> tNUMBER
%token tLBRACE tRBRACE tLPAREN tRPAREN tADD tMINUS tMUL tDIV tGT tLT tSET tEQL tCOMMA tSEMICOLON tEOL tIF tELSE tDEF
%token<str> tIDENTIFIER

%type<ast> program statements statement block expression primary arguments

%left tGT tLT
%left tADD tMINUS
%left tMUL tDIV

%%

program:
      statements { ast = (TopAST*)$$; }

statements:
      statements tEOL { $$ = $1; }
    | statements tSEMICOLON { $$ = $1; }
    | statement { $$ = new BlockAST($1); }
    | statements statement { $$->addChild($2); }

statement:
      { $$ = NULL; }
    | tDEF tIDENTIFIER tLPAREN arguments tRPAREN block { $$ = new DefAST(*$2, $4, $6); }
    | tIF expression block { $$ = new IfAST($2, $3); }
    | tIF expression block tELSE block { $$ = new IfAST($2, $3, $5); }
    | expression { $$ = $1; }

block:
      tLBRACE statements tRBRACE { $$ = $2; }

expression:
      primary { $$ = $1; }
    | primary tSET expression { $$ = new BinaryExprAST("=", $1, $3); }
    | tMINUS expression { $$ = new BinaryExprAST("-", $2); }
    | expression tGT expression { $$ = new BinaryExprAST(">", $1, $3); }
    | expression tLT expression { $$ = new BinaryExprAST("<", $1, $3); }
    | expression tADD expression { $$ = new BinaryExprAST("+", $1, $3); }
    | expression tMINUS expression { $$ = new BinaryExprAST("-", $1, $3); }
    | expression tMUL expression { $$ = new BinaryExprAST("*", $1, $3); }
    | expression tDIV expression { $$ = new BinaryExprAST("/", $1, $3); }
    | tLPAREN expression tRPAREN { $$ = $2; }

primary:
      tNUMBER { $$ = new ASTLeaf(new NumberToken($1)); }
    | tIDENTIFIER { $$ = new ValuableAST(*$1); }
    | tIDENTIFIER tLPAREN arguments tRPAREN { $$ = new CallFunctionAST(*$1, $3); }

arguments:
      { $$ = new ArgumentsAST(); }
    | expression { $$ = new ArgumentsAST($1); }
    | arguments tCOMMA expression { $$->addChild($3); }

%%
