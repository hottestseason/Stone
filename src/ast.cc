#include <iostream>
#include <typeinfo>
#include "token.h"
#include "ast.h"
#include "ast_visitor.h"

AST::AST() {
    children = new std::vector<AST*>;
}

AST::AST(AST *ast) {
    children = new std::vector<AST*>;
    add(ast);
}

AST::AST(AST *lAst, AST *rAST) {
    children = new std::vector<AST*>;
    add(lAst);
    add(rAST);
}

std::vector<AST*>* AST::getChildren() const {
    return children;
}

AST* AST::get(int i) const {
    return (*children)[i];
}

void AST::add(AST *ast) {
    if (ast) {
        children->push_back(ast);
    }
}

void AST::print(std::ostream &out) const {
    out << "( ";
    for (AST* child : *children) {
        out << *child << " ";
    }
    out << ")";
}

std::ostream& operator<<(std::ostream &out, const AST &ast) {
    ast.print(out);
    return out;
}

ASTLeaf::ASTLeaf() {}
ASTLeaf::ASTLeaf(Token *token) : token(token) {}

Token* ASTLeaf::getToken() const {
    return token;
}

void ASTLeaf::print(std::ostream &out) const {
    if (token->isInteger()) {
        out << token->getInteger();
    } else if (token->isDouble()) {
        out << token->getDouble();
    } else if (token->isIdentifier()) {
        out << token->getText();
    }
}

void ASTLeaf::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

VariableAST::VariableAST(std::string name) : AST() {
    add(new ASTLeaf(new IdentifierToken(name)));
}

VariableAST::VariableAST(std::string name, std::string type) : AST() {
    add(new ASTLeaf(new IdentifierToken(name)));
    add(new ASTLeaf(new IdentifierToken(type)));
}

void VariableAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

std::string VariableAST::getName() {
    return dynamic_cast<ASTLeaf*>(get(0))->getToken()->getText();
}

std::string VariableAST::getTypeName() {
    return dynamic_cast<ASTLeaf*>(get(1))->getToken()->getText();
}

BinaryExprAST::BinaryExprAST(std::string tokenName, AST *ast) : AST() {
    Token* token = new IdentifierToken(tokenName);
    ASTLeaf* op = new ASTLeaf(token);
    add(op);
    add(ast);
}

BinaryExprAST::BinaryExprAST(std::string tokenName, AST *lAst, AST *rAst) : AST() {
    Token* token = new IdentifierToken(tokenName);
    ASTLeaf* op = new ASTLeaf(token);
    add(op);
    add(lAst);
    add(rAst);
}

std::string BinaryExprAST::op() {
    return dynamic_cast<ASTLeaf*>(get(0))->getToken()->getText();
}

AST* BinaryExprAST::left() {
    return get(1);
}

AST* BinaryExprAST::right() {
    return get(2);
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
    return children->size();
}

VariableAST *ArgumentsAST::get(int i) {
    return dynamic_cast<VariableAST*>(AST::get(i));
}

CallFunctionAST::CallFunctionAST(std::string name, AST *args) : AST() {
    Token* token = new IdentifierToken(name);
    ASTLeaf* funName = new ASTLeaf(token);
    add(funName);
    add(args);
}

void CallFunctionAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

std::string CallFunctionAST::name() {
    return dynamic_cast<ASTLeaf*>(get(0))->getToken()->getText();
}

ArgumentsAST* CallFunctionAST::arguments() {
    return dynamic_cast<ArgumentsAST*>(get(1));
}

IfAST::IfAST(AST *expr, AST *block) : AST(expr, block) {
}

IfAST::IfAST(AST *expr, AST *thenBlock, AST *elseBlock) : AST(expr, thenBlock) {
    add(elseBlock);
}

void IfAST::print(std::ostream &out) const {
    out << "( if " << *condition() << " then " << *thenBlock() << " else ";
    if (elseBlock()) {
        out << *elseBlock();
    }
    out << " )";
}

AST* IfAST::condition() const {
    return get(0);
}

AST* IfAST::thenBlock() const {
    return get(1);
}

AST* IfAST::elseBlock() const {
    return get(2);
}

void IfAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

DefAST::DefAST(std::string name, AST *body, std::string typeName) {
    add(new ASTLeaf(new IdentifierToken(name)));
    add(new ArgumentsAST());
    add(body);
    add(new ASTLeaf(new IdentifierToken(typeName)));
}

DefAST::DefAST(std::string name, AST *args, AST *body, std::string typeName) : AST() {
    add(new ASTLeaf(new IdentifierToken(name)));
    add(args);
    add(body);
    add(new ASTLeaf(new IdentifierToken(typeName)));
}

void DefAST::print(std::ostream &out) const {
    out << "( def " << name() << *arguments() << " " << *body() << " )";
}

std::string DefAST::name() const {
    return dynamic_cast<ASTLeaf *>(get(0))->getToken()->getText();
}

ArgumentsAST* DefAST::arguments() const {
    return dynamic_cast<ArgumentsAST*>(get(1));
}

AST* DefAST::body() const {
    return get(2);
}

std::string DefAST::getTypeName() {
    return dynamic_cast<ASTLeaf *>(get(3))->getToken()->getText();;
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
