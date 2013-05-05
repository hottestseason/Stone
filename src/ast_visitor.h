#pragma once
#include "ast.h"

class ASTVisitor {
public:
    ASTVisitor();
    virtual void visit(ASTLeaf*) = 0;
    virtual void visit(BinaryExprAST*) = 0;
    virtual void visit(ArgumentsAST*) = 0;
    virtual void visit(CallFunctionAST*) = 0;
    virtual void visit(IfAST*) = 0;
    virtual void visit(DefAST*) = 0;
    virtual void visit(TopAST*) = 0;
    virtual void visit(BlockAST*) = 0;
    virtual void visit(ValuableAST*) = 0;
    virtual ~ASTVisitor();
};
