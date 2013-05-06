#pragma once
#include "llvm.h"
#include "ast.h"
#include "ast_visitor.h"

class CodeGenerator : ASTVisitor {
public:
    CodeGenerator();
    ~CodeGenerator();

    void execute(TopAST*);
    void visit(AST*);
    void visit(ASTLeaf*);
    void visit(BinaryExprAST*);
    void visit(ArgumentsAST*);
    void visit(CallFunctionAST*);
    void visit(IfAST*);
    void visit(DefAST*);
    void visit(TopAST*);
    void visit(BlockAST*);
    void visit(VariableAST*);
    void dump();
    void error(const char *);

private:
    llvm::Module *module;
    llvm::IRBuilder<> *builder;
    llvm::Value *lastValue;
    std::map<std::string, llvm::AllocaInst*> *namedValues;
    llvm::ExecutionEngine *executionEngine;

    void visitChildren(AST*);
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function*, VariableAST*);
    void setFunctionArguments(llvm::Function *, ArgumentsAST*);
    llvm::Type *getType(const std::string&);
    llvm::Type *getType(DefAST*);
    std::vector<llvm::Type*> *createArgTypes(ArgumentsAST*);
};
