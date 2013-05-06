#include <stdio.h>
#include <iostream>
#include "ast.h"
#include "parse.hh"
#include "code_generator.h"

using namespace std;

extern "C" {
    int yyparse();
    int yylex();
}
extern FILE * yyin;

extern TopAST *ast;

int main(int argc, char *argv[]) {
    if ( argc > 1 ) {
        yyin = fopen(argv[1], "r");
    } else {
        yyin = stdin;
	}
    yyparse();
    cout << *ast << endl;
    CodeGenerator generator;
    generator.execute(ast);
    return 0;
}
