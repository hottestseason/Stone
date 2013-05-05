#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include "code_generator.h"

CodeGenerator::CodeGenerator(TopAST *top) {
    llvm::InitializeNativeTarget();
    mTop = top;
    mModule = new llvm::Module("top", llvm::getGlobalContext());
    mBuilder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    mNamedValues = new std::map<std::string, llvm::Value*>;
    executionEngine = llvm::EngineBuilder(mModule).create();
}

CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::generate() {
    visit(mTop);
}

void CodeGenerator::visit(ASTLeaf *ast) {
    if (ast->token()->isNumber()) {
        mValue = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(ast->token()->number()));
    } else {
        mValue = (*mNamedValues)[ast->token()->text()];
    }
}

void CodeGenerator::visit(BinaryExprAST *ast) {
    ast->left()->accept(this);
    auto lValue = mValue;
    ast->right()->accept(this);
    auto rValue = mValue;

    if (ast->op() == "+") {
        mValue = mBuilder->CreateFAdd(lValue, rValue);
    } else if (ast->op() == "-") {
        mValue = mBuilder->CreateFSub(lValue, rValue);
    } else if(ast->op() == ">") {
        mValue = mBuilder->CreateUIToFP(mBuilder->CreateFCmpUGT(lValue, rValue), llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    }
}

void CodeGenerator::visit(ArgumentsAST *ast) {
    error("shoudn't be called");
}

void CodeGenerator::visit(CallFunctionAST *ast) {
    auto function = mModule->getFunction(ast->name());
    std::vector<llvm::Value*> argValues;
    for (AST* arg : *ast->arguments()->children()) {
        arg->accept(this);
        argValues.push_back(mValue);
    }
    mValue = mBuilder->CreateCall(function, argValues);
}

void CodeGenerator::visit(IfAST *ast) {
    ast->condition()->accept(this);
    auto condValue = mBuilder->CreateFCmpONE(mValue, llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(0.0)));

    auto currentFunction = mBuilder->GetInsertBlock()->getParent();
    auto thenBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", currentFunction);
    auto elseBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
    auto mergeBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "merge");
    mBuilder->CreateCondBr(condValue, thenBlock, elseBlock);

    mBuilder->SetInsertPoint(thenBlock);
    ast->thenBlock()->accept(this);
    auto thenValue = mValue;

    mBuilder->CreateBr(mergeBlock);
    thenBlock = mBuilder->GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(elseBlock);
    mBuilder->SetInsertPoint(elseBlock);
    ast->elseBlock()->accept(this);
    auto elseValue = mValue;

    mBuilder->CreateBr(mergeBlock);
    elseBlock = mBuilder->GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(mergeBlock);
    mBuilder->SetInsertPoint(mergeBlock);
    auto phiNode = mBuilder->CreatePHI(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 2);
    phiNode->addIncoming(thenValue, thenBlock);
    phiNode->addIncoming(elseValue, elseBlock);

    mValue = phiNode;
}

void CodeGenerator::visit(DefAST *ast) {
    mNamedValues->clear();
    std::vector<llvm::Type*> argTypes(ast->arguments()->size(), llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    auto *functionType = llvm::FunctionType::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), argTypes, false);
    lastFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ast->name(), mModule);
    int i = 0;
    for (auto argIterator = lastFunction->arg_begin(); i != lastFunction->arg_size(); ++argIterator, ++i) {
        argIterator->setName(ast->arguments()->name(i));
        (*mNamedValues)[ast->arguments()->name(i)] = argIterator;
    }
    auto *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", lastFunction);
    mBuilder->SetInsertPoint(block);
    ast->body()->accept(this);
    mBuilder->CreateRet(mValue);
}

void CodeGenerator::visit(TopAST *ast) {
    for (AST* child : *ast->children()) {
        if (typeid(*child) == typeid(DefAST)) {
            child->accept(this);
            lastFunction->dump();
        } else {
            (new DefAST("", child))->accept(this);
            lastFunction->dump();
            double (*fp)() = (double (*)())(intptr_t)executionEngine->getPointerToFunction(lastFunction);
            std::cout << "Evaluated to " << fp() << std::endl;
        }
    }
}

void CodeGenerator::visit(BlockAST *ast) {
    visitChildren(ast);
}

void CodeGenerator::dump() {
    mModule->dump();
}

void CodeGenerator::error(const char *str) {
    std::cerr << "Error: " << str << std::endl;
}

void CodeGenerator::visitChildren(AST* ast) {
    for (AST* child : *ast->children()) {
        child->accept(this);
    }
}
