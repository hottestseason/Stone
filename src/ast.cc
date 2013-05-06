#include <iostream>
#include <typeinfo>
#include "token.h"
#include "ast.h"
#include "ast_visitor.h"

AST::AST() {
    mChildren = new std::vector<AST*>;
}

AST::AST(AST *ast) {
    mChildren = new std::vector<AST*>;
    addChild(ast);
}

AST::AST(AST *lAst, AST *rAST) {
    mChildren = new std::vector<AST*>;
    addChild(lAst);
    addChild(rAST);
}

std::vector<AST*>* AST::children() const {
    return mChildren;
}

AST* AST::child(int i) const {
    if (children()->size() > i) {
        return children()->at(i);
    } else {
        return NULL;
    }
}

void AST::addChild(AST *ast) {
    if (ast) {
        children()->push_back(ast);
    }
}

void AST::print(std::ostream &out) const {
    out << "( ";
    for (AST* child : *children()) {
        out << *child << " ";
    }
    out << ")";
}

std::ostream& operator<<(std::ostream &out, const AST &ast) {
    ast.print(out);
    return out;
}

ASTLeaf::ASTLeaf(Token *token) {
    mToken = token;
}

Token* ASTLeaf::token() const {
    return mToken;
}

void ASTLeaf::print(std::ostream &out) const {
    if (token()->isInteger()) {
        out << token()->getInteger();
    } else if (token()->isDouble()) {
        out << token()->getDouble();
    } else if (token()->isIdentifier()) {
        out << token()->getText();
    }
}

void ASTLeaf::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

VariableAST::VariableAST(std::string name) : AST() {
    addChild(new ASTLeaf(new IdentifierToken(name)));
}

VariableAST::VariableAST(std::string name, std::string type) : AST() {
    addChild(new ASTLeaf(new IdentifierToken(name)));
    addChild(new ASTLeaf(new IdentifierToken(type)));
}

void VariableAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

std::string VariableAST::getName() {
    return dynamic_cast<ASTLeaf*>(child(0))->token()->getText();
}

std::string VariableAST::getTypeName() {
    return dynamic_cast<ASTLeaf*>(child(1))->token()->getText();
}

BinaryExprAST::BinaryExprAST(std::string tokenName, AST *ast) : AST() {
    Token* token = new IdentifierToken(tokenName);
    ASTLeaf* op = new ASTLeaf(token);
    addChild(op);
    addChild(ast);
}

BinaryExprAST::BinaryExprAST(std::string tokenName, AST *lAst, AST *rAst) : AST() {
    Token* token = new IdentifierToken(tokenName);
    ASTLeaf* op = new ASTLeaf(token);
    addChild(op);
    addChild(lAst);
    addChild(rAst);
}

std::string BinaryExprAST::op() {
    return dynamic_cast<ASTLeaf*>(child(0))->token()->getText();
}

AST* BinaryExprAST::left() {
    return child(1);
}

AST* BinaryExprAST::right() {
    return child(2);
}

void BinaryExprAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

ArgumentsAST::ArgumentsAST() : AST() {
}

ArgumentsAST::ArgumentsAST(AST *arg) : AST(arg) {
}

void ArgumentsAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

int ArgumentsAST::size() {
    return children()->size();
}

VariableAST *ArgumentsAST::get(int i) {
    return dynamic_cast<VariableAST*>(child(i));
}

CallFunctionAST::CallFunctionAST(std::string name, AST *args) : AST() {
    Token* token = new IdentifierToken(name);
    ASTLeaf* funName = new ASTLeaf(token);
    addChild(funName);
    addChild(args);
}

void CallFunctionAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

std::string CallFunctionAST::name() {
    return dynamic_cast<ASTLeaf*>(child(0))->token()->getText();
}

ArgumentsAST* CallFunctionAST::arguments() {
    return dynamic_cast<ArgumentsAST*>(child(1));
}

IfAST::IfAST(AST *expr, AST *block) : AST(expr, block) {
}

IfAST::IfAST(AST *expr, AST *thenBlock, AST *elseBlock) : AST(expr, thenBlock) {
    addChild(elseBlock);
}

void IfAST::print(std::ostream &out) const {
    out << "( if " << *condition() << " then " << *thenBlock() << " else ";
    if (elseBlock()) {
        out << *elseBlock();
    }
    out << " )";
}

AST* IfAST::condition() const {
    return child(0);
}

AST* IfAST::thenBlock() const {
    return child(1);
}

AST* IfAST::elseBlock() const {
    return child(2);
}

void IfAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

DefAST::DefAST(std::string name, AST *body, std::string typeName) {
    addChild(new ASTLeaf(new IdentifierToken(name)));
    addChild(new ArgumentsAST());
    addChild(body);
    addChild(new ASTLeaf(new IdentifierToken(typeName)));
}

DefAST::DefAST(std::string name, AST *args, AST *body, std::string typeName) : AST() {
    addChild(new ASTLeaf(new IdentifierToken(name)));
    addChild(args);
    addChild(body);
    addChild(new ASTLeaf(new IdentifierToken(typeName)));
}

void DefAST::print(std::ostream &out) const {
    out << "( def " << name() << *arguments() << " " << *body() << " )";
}

std::string DefAST::name() const {
    return dynamic_cast<ASTLeaf *>(child(0))->token()->getText();
}

ArgumentsAST* DefAST::arguments() const {
    return dynamic_cast<ArgumentsAST*>(child(1));
}

AST* DefAST::body() const {
    return child(2);
}

std::string DefAST::getTypeName() {
    return dynamic_cast<ASTLeaf *>(child(3))->token()->getText();;
}

void DefAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

void TopAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

BlockAST::BlockAST(AST *ast) : AST(ast) {};

void BlockAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}
