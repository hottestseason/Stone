#include <iostream>
#include <typeinfo>
#include "token.h"
#include "ast.h"
#include "ast_visitor.h"

AST::AST() {
    mChildren = new vector<AST*>;
}

AST::AST(AST *ast) {
    mChildren = new vector<AST*>;
    addChild(ast);
}

AST::AST(AST *lAst, AST *rAST) {
    mChildren = new vector<AST*>;
    addChild(lAst);
    addChild(rAST);
}

vector<AST*>* AST::children() const {
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

void AST::print(ostream &out) const {
    out << "( ";
    for (AST* child : *children()) {
        out << *child << " ";
    }
    out << ")";
}

ostream& operator<<(ostream &out, const AST &ast) {
    ast.print(out);
    return out;
}

ASTLeaf::ASTLeaf(Token *token) {
    mToken = token;
}

Token* ASTLeaf::token() const {
    return mToken;
}

void ASTLeaf::print(ostream &out) const {
    if (token()->isNumber()) {
        out << token()->number();
    } else if (token()->isIdentifier()) {
        out << token()->text();
    }
}

void ASTLeaf::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

ValuableAST::ValuableAST(std::string name) : AST() {
    addChild(new ASTLeaf(new IdentifierToken(name)));
}

void ValuableAST::accept(ASTVisitor *visitor) {
    visitor->visit(this);
}

std::string ValuableAST::getName() {
    return dynamic_cast<ASTLeaf*>(child(0))->token()->text();
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
    return dynamic_cast<ASTLeaf*>(child(0))->token()->text();
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

ValuableAST *ArgumentsAST::get(int i) {
    return dynamic_cast<ValuableAST*>(child(i));
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
    return dynamic_cast<ASTLeaf*>(child(0))->token()->text();
}

ArgumentsAST* CallFunctionAST::arguments() {
    return dynamic_cast<ArgumentsAST*>(child(1));
}

IfAST::IfAST(AST *expr, AST *block) : AST(expr, block) {
}

IfAST::IfAST(AST *expr, AST *thenBlock, AST *elseBlock) : AST(expr, thenBlock) {
    addChild(elseBlock);
}

void IfAST::print(ostream &out) const {
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

DefAST::DefAST(std::string name, AST *body) {
    addChild(new ASTLeaf(new IdentifierToken(name)));
    addChild(new ArgumentsAST());
    addChild(body);
}

DefAST::DefAST(std::string name, AST *args, AST *body) : AST() {
    addChild(new ASTLeaf(new IdentifierToken(name)));
    addChild(args);
    addChild(body);
}

void DefAST::print(ostream &out) const {
    out << "( def " << name() << *arguments() << " " << *body() << " )";
}

std::string DefAST::name() const {
    return dynamic_cast<ASTLeaf *>(child(0))->token()->text();
}

ArgumentsAST* DefAST::arguments() const {
    return dynamic_cast<ArgumentsAST*>(child(1));
}

AST* DefAST::body() const {
    return child(2);
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
