#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include "code_generator.h"

CodeGenerator::CodeGenerator(TopAST *topAst) {
    llvm::InitializeNativeTarget();
    this->topAst = topAst;
    module = new llvm::Module("top", llvm::getGlobalContext());
    builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    namedValues = new std::map<std::string, llvm::AllocaInst*>;
    executionEngine = llvm::EngineBuilder(module).create();
}

CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::generate() {
    visit(topAst);
}

void CodeGenerator::visit(ASTLeaf *ast) {
    if (ast->token()->isNumber()) {
        lastValue = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(ast->token()->number()));
    } else {
        auto varName = ast->token()->text();
        lastValue = builder->CreateLoad((*namedValues)[varName], varName);
    }
}

void CodeGenerator::visit(BinaryExprAST *ast) {
    ast->left()->accept(this);
    auto lValue = lastValue;
    ast->right()->accept(this);
    auto rValue = lastValue;

    if (ast->op() == "+") {
        lastValue = builder->CreateFAdd(lValue, rValue);
    } else if (ast->op() == "-") {
        lastValue = builder->CreateFSub(lValue, rValue);
    } else if(ast->op() == ">") {
        lastValue = builder->CreateUIToFP(builder->CreateFCmpUGT(lValue, rValue), llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    }
}

void CodeGenerator::visit(ArgumentsAST *ast) {
    error("shoudn't be called");
}

void CodeGenerator::visit(CallFunctionAST *ast) {
    auto function = module->getFunction(ast->name());
    std::vector<llvm::Value*> argValues;
    for (AST* arg : *ast->arguments()->children()) {
        arg->accept(this);
        argValues.push_back(lastValue);
    }
    lastValue = builder->CreateCall(function, argValues);
}

void CodeGenerator::visit(IfAST *ast) {
    ast->condition()->accept(this);
    auto condValue = builder->CreateFCmpONE(lastValue, llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(0.0)));

    auto currentFunction = builder->GetInsertBlock()->getParent();
    auto thenBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", currentFunction);
    auto elseBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
    auto mergeBlock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "merge");
    builder->CreateCondBr(condValue, thenBlock, elseBlock);

    builder->SetInsertPoint(thenBlock);
    ast->thenBlock()->accept(this);
    auto thenValue = lastValue;

    builder->CreateBr(mergeBlock);
    thenBlock = builder->GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(elseBlock);
    builder->SetInsertPoint(elseBlock);
    ast->elseBlock()->accept(this);
    auto elseValue = lastValue;

    builder->CreateBr(mergeBlock);
    elseBlock = builder->GetInsertBlock();

    currentFunction->getBasicBlockList().push_back(mergeBlock);
    builder->SetInsertPoint(mergeBlock);
    auto phiNode = builder->CreatePHI(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 2);
    phiNode->addIncoming(thenValue, thenBlock);
    phiNode->addIncoming(elseValue, elseBlock);

    lastValue = phiNode;
}

void CodeGenerator::visit(DefAST *ast) {
    namedValues->clear();
    std::vector<llvm::Type*> argTypes(ast->arguments()->size(), llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    auto *functionType = llvm::FunctionType::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), argTypes, false);
    auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ast->name(), module);

    int i = 0;
    for (auto argIterator = function->arg_begin(); i != function->arg_size(); ++argIterator, ++i) {
        argIterator->setName(ast->arguments()->name(i));
    }

    auto *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    builder->SetInsertPoint(block);

    createArgumentAllocas(function, ast->arguments());

    ast->body()->accept(this);
    builder->CreateRet(lastValue);

    lastValue = function;
}

void CodeGenerator::visit(TopAST *ast) {
    for (AST* child : *ast->children()) {
        if (typeid(*child) == typeid(DefAST)) {
            child->accept(this);
            lastValue->dump();
        } else {
            (new DefAST("", child))->accept(this);
            lastValue->dump();
            double (*fp)() = (double (*)())(intptr_t)executionEngine->getPointerToFunction(dynamic_cast<llvm::Function*>(lastValue));
            std::cout << "Evaluated to " << fp() << std::endl;
        }
    }
}

void CodeGenerator::visit(BlockAST *ast) {
    visitChildren(ast);
}

void CodeGenerator::dump() {
    module->dump();
}

void CodeGenerator::error(const char *str) {
    std::cerr << "Error: " << str << std::endl;
}

void CodeGenerator::visitChildren(AST* ast) {
    for (AST* child : *ast->children()) {
        child->accept(this);
    }
}

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *function, const std::string &varName) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 0, varName);
}

void CodeGenerator::createArgumentAllocas(llvm::Function *function, ArgumentsAST *arguments) {
    int i = 0;
    for (auto argIterator = function->arg_begin(); i != function->arg_size(); ++argIterator, ++i) {
        auto alloca = createEntryBlockAlloca(function, arguments->name(i));
        builder->CreateStore(argIterator, alloca);
        (*namedValues)[arguments->name(i)] = alloca;
    }
}
