#pragma once
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/IRBuilder.h"
#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/DataLayout.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/TargetSelect.h"
#include "ast.h"
#include "ast_visitor.h"

class CodeGenerator : ASTVisitor {
public:
    CodeGenerator(TopAST*);
    ~CodeGenerator();

    void execute();
    void generate();
    void visit(AST*);
    void visit(ASTLeaf*);
    void visit(BinaryExprAST*);
    void visit(ArgumentsAST*);
    void visit(CallFunctionAST*);
    void visit(IfAST*);
    void visit(DefAST*);
    void visit(TopAST*);
    void visit(BlockAST*);
    void dump();
    void error(const char *);

private:
    TopAST *topAst;
    llvm::Module *module;
    llvm::IRBuilder<> *builder;
    llvm::Value *lastValue;
    std::map<std::string, llvm::AllocaInst*> *namedValues;
    llvm::ExecutionEngine *executionEngine;

    void visitChildren(AST*);
    llvm::AllocaInst *createEntryBlockAlloca(llvm::Function*, const std::string&);
    void createArgumentAllocas(llvm::Function *, ArgumentsAST*);
};
