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
    if (ast->token()->isInteger()) {
        lastValue = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, ast->token()->getInteger()));
    } else if (ast->token()->isDouble()) {
        lastValue = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(ast->token()->getDouble()));
    }
}

void CodeGenerator::visit(BinaryExprAST *ast) {
    if (ast->op() == "=") {
        auto valuable = dynamic_cast<ValuableAST*>(ast->left());
        ast->right()->accept(this);
        auto rValue = lastValue;
        if (!(*namedValues)[valuable->getName()]) {
            auto alloca = createEntryBlockAlloca(builder->GetInsertBlock()->getParent(), valuable);
            (*namedValues)[valuable->getName()] = alloca;
        }
        builder->CreateStore(rValue, (*namedValues)[valuable->getName()]);
    } else {
        ast->left()->accept(this);
        auto lValue = lastValue;
        ast->right()->accept(this);
        auto rValue = lastValue;

        if (ast->op() == "+") {
            if (lValue->getType()->isDoubleTy() || rValue->getType()->isDoubleTy()) {
                if (lValue->getType()->isIntegerTy()) {
                    lValue = builder->CreateSIToFP(lValue, getType("double"));
                }
                if (rValue->getType()->isIntegerTy()) {
                    rValue = builder->CreateSIToFP(rValue, getType("double"));
                }
                lastValue = builder->CreateFAdd(lValue, rValue);
            } else {
                lastValue = builder->CreateAdd(lValue, rValue);
            }
        } else if (ast->op() == "-") {
            if (lValue->getType()->isDoubleTy() || rValue->getType()->isDoubleTy()) {
                if (lValue->getType()->isIntegerTy()) {
                    lValue = builder->CreateSIToFP(lValue, getType("double"));
                }
                if (rValue->getType()->isIntegerTy()) {
                    rValue = builder->CreateSIToFP(rValue, getType("double"));
                }
                lastValue = builder->CreateFSub(lValue, rValue);
            } else {
                lastValue = builder->CreateSub(lValue, rValue);
            }
        } else if(ast->op() == ">") {
            if (lValue->getType()->isDoubleTy() || rValue->getType()->isDoubleTy()) {
                if (lValue->getType()->isIntegerTy()) {
                    lValue = builder->CreateSIToFP(lValue, getType("double"));
                }
                if (rValue->getType()->isIntegerTy()) {
                    rValue = builder->CreateSIToFP(rValue, getType("double"));
                }
                lastValue = builder->CreateFCmpUGT(lValue, rValue);
            } else {
                lastValue = builder->CreateICmpUGT(lValue, rValue);
            }
        }
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
    auto condValue = lastValue;

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
    auto phiNode = builder->CreatePHI(thenValue->getType(), 2);
    phiNode->addIncoming(thenValue, thenBlock);
    phiNode->addIncoming(elseValue, elseBlock);

    lastValue = phiNode;
}

void CodeGenerator::visit(DefAST *ast) {
    namedValues->clear();
    auto argTypes = createArgTypes(ast->arguments());
    auto functionReturnType = getType(ast->getTypeName());
    if (!functionReturnType) {
        functionReturnType = getType(ast);
    }
    auto *functionType = llvm::FunctionType::get(functionReturnType, *argTypes, false);
    auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ast->name(), module);

    auto *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    builder->SetInsertPoint(block);

    setFunctionArguments(function, ast->arguments());

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
            (new DefAST("", child, ""))->accept(this);
            auto function = dynamic_cast<llvm::Function*>(lastValue);
            function->dump();
            std::cout << "Evaluated to ";
            if (function->getReturnType()->isIntegerTy()) {
                int (*fp)() = (int (*)())(intptr_t)executionEngine->getPointerToFunction(function);
                std::cout << fp();
            } else if (function->getReturnType()->isDoubleTy()) {
                double (*fp)() = (double (*)())(intptr_t)executionEngine->getPointerToFunction(function);
                std::cout <<  fp();
            }
            std::cout <<  std::endl;
        }
    }
}

void CodeGenerator::visit(BlockAST *ast) {
    visitChildren(ast);
}

void CodeGenerator::visit(ValuableAST *ast) {
    lastValue = builder->CreateLoad((*namedValues)[ast->getName()]);
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

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *function, ValuableAST *valuable) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(getType(valuable->getTypeName()), 0, valuable->getName());
}

void CodeGenerator::setFunctionArguments(llvm::Function *function, ArgumentsAST *arguments) {
    int i = 0;
    for (auto argIterator = function->arg_begin(); i != function->arg_size(); ++argIterator, ++i) {
        argIterator->setName(arguments->get(i)->getName());
    }
    i = 0;
    for (auto argIterator = function->arg_begin(); i != function->arg_size(); ++argIterator, ++i) {
        auto arg = arguments->get(i);
        auto alloca = createEntryBlockAlloca(function, arg);
        builder->CreateStore(argIterator, alloca);
        (*namedValues)[arg->getName()] = alloca;
    }
}

llvm::Type *CodeGenerator::getType(const std::string &type) {
    if (type == "int") {
        return llvm::Type::getInt64Ty(llvm::getGlobalContext());
    } else if (type == "double") {
        return llvm::Type::getDoubleTy(llvm::getGlobalContext());
    } else if (type == "void") {
        return llvm::Type::getVoidTy(llvm::getGlobalContext());
    } else {
        return NULL;
    }
}

llvm::Type *CodeGenerator::getType(DefAST *ast) {
    namedValues->clear();
    auto argTypes = createArgTypes(ast->arguments());

    auto *functionType = llvm::FunctionType::get(getType("void"), *argTypes, false);
    auto function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, ast->name(), module);

    auto *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    builder->SetInsertPoint(block);

    setFunctionArguments(function, ast->arguments());

    ast->body()->accept(this);
    function->eraseFromParent();
    return lastValue->getType();
}

std::vector<llvm::Type*> *CodeGenerator::createArgTypes(ArgumentsAST *args) {
    auto argTypes = new std::vector<llvm::Type*>;
    for (int i = 0; i < args->size(); i++) {
        argTypes->push_back(getType(args->get(i)->getTypeName()));
    }
    return argTypes;
}
