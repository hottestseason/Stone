#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstdio>
#include "code_generator.h"

CodeGenerator::CodeGenerator() {
    llvm::InitializeNativeTarget();
    module = new llvm::Module("top", llvm::getGlobalContext());
    builder = new llvm::IRBuilder<>(llvm::getGlobalContext());
    namedValues = new std::map<std::string, llvm::AllocaInst*>;
    executionEngine = llvm::EngineBuilder(module).create();
    functionPassManager = new llvm::FunctionPassManager(module);
    functionPassManager->add(new llvm::DataLayout(*executionEngine->getDataLayout()));
    functionPassManager->add(llvm::createBasicAliasAnalysisPass());
    functionPassManager->add(llvm::createInstructionCombiningPass());
    functionPassManager->add(llvm::createReassociatePass());
    functionPassManager->add(llvm::createGVNPass());
    functionPassManager->add(llvm::createCFGSimplificationPass());
    functionPassManager->doInitialization();
}

CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::execute(TopAST *topAst) {
    visit(topAst);
}

void CodeGenerator::visit(ASTLeaf *ast) {
    Token *token = ast->getToken();
    if (token->isInteger()) {
        lastValue = llvm::ConstantInt::get(llvm::getGlobalContext(), llvm::APInt(64, token->getInteger()));
    } else if (token->isDouble()) {
        lastValue = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(token->getDouble()));
    }
}

void CodeGenerator::visit(BinaryExprAST *ast) {
    if (ast->op() == "=") {
        auto variable = dynamic_cast<VariableAST*>(ast->left());
        ast->right()->accept(this);
        auto rValue = lastValue;
        if (!(*namedValues)[variable->getName()]) {
            auto alloca = createEntryBlockAlloca(builder->GetInsertBlock()->getParent(), variable);
            (*namedValues)[variable->getName()] = alloca;
        }
        builder->CreateStore(rValue, (*namedValues)[variable->getName()]);
    } else {
        ast->left()->accept(this);
        auto lValue = lastValue;
        ast->right()->accept(this);
        auto rValue = lastValue;

        if (ast->op() == "+" || ast->op() == "-" || ast->op() == "*" || ast->op() == "/" || ast->op() == ">" || ast->op() == "<") {
            if (lValue->getType()->isDoubleTy() || rValue->getType()->isDoubleTy()) {
                if (lValue->getType()->isIntegerTy()) {
                    lValue = builder->CreateSIToFP(lValue, getType("double"));
                }
                if (rValue->getType()->isIntegerTy()) {
                    rValue = builder->CreateSIToFP(rValue, getType("double"));
                }
                if (ast->op() == "+") {
                    lastValue = builder->CreateFAdd(lValue, rValue);
                } else if (ast->op() == "-") {
                    lastValue = builder->CreateFSub(lValue, rValue);
                } else if (ast->op() == "*") {
                    lastValue = builder->CreateFMul(lValue, rValue);
                } else if (ast->op() == "/") {
                    lastValue = builder->CreateFDiv(lValue, rValue);
                } else if (ast->op() == ">") {
                    lastValue = builder->CreateFCmpOGT(lValue, rValue);
                } else if (ast->op() == "<") {
                    lastValue = builder->CreateFCmpOLT(lValue, rValue);
                }
            } else {
                if (ast->op() == "+") {
                    lastValue = builder->CreateAdd(lValue, rValue);
                } else if (ast->op() == "-") {
                    lastValue = builder->CreateSub(lValue, rValue);
                } else if (ast->op() == "*") {
                    lastValue = builder->CreateMul(lValue, rValue);
                } else if (ast->op() == "/") {
                    lastValue = builder->CreateSDiv(lValue, rValue);
                } else if (ast->op() == ">") {
                    lastValue = builder->CreateICmpSGT(lValue, rValue);
                } else if (ast->op() == "<") {
                    lastValue = builder->CreateICmpSLT(lValue, rValue);
                }
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
    for (AST* arg : *ast->arguments()->getChildren()) {
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

    functionPassManager->run(*function);

    lastValue = function;
}

void CodeGenerator::visit(TopAST *ast) {
    for (AST* child : *ast->getChildren()) {
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

void CodeGenerator::visit(VariableAST *ast) {
    lastValue = builder->CreateLoad((*namedValues)[ast->getName()]);
}

void CodeGenerator::error(const char *str) {
    std::cerr << "Error: " << str << std::endl;
}

void CodeGenerator::visitChildren(AST* ast) {
    for (AST* child : *ast->getChildren()) {
        child->accept(this);
    }
}

llvm::AllocaInst *CodeGenerator::createEntryBlockAlloca(llvm::Function *function, VariableAST *variable) {
    llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpBuilder.CreateAlloca(getType(variable->getTypeName()), 0, variable->getName());
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
