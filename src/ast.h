#pragma once
#include <vector>
#include "token.h"

class ASTVisitor;

class AST {
public:
    AST();
    AST(AST*);
    AST(AST*, AST*);
    std::vector<AST*>* children() const;
    AST* child(int) const;
    void addChild(AST*);
    virtual void print(ostream&) const;
    virtual void accept(ASTVisitor*) = 0;
    friend ostream& operator<<(ostream&, const AST&);
private:
    vector<AST*>* mChildren;
};

class ASTLeaf : public AST {
public:
    ASTLeaf(Token*);
    Token* token() const;
    void print(ostream&) const;
    void accept(ASTVisitor*);
private:
    Token* mToken;
};

class BinaryExprAST : public AST {
public:
    BinaryExprAST(string, AST*);
    BinaryExprAST(string, AST*, AST*);
    string op();
    AST* left();
    AST* right();
    void accept(ASTVisitor*);
};

class ArgumentsAST : public AST {
public:
    ArgumentsAST();
    ArgumentsAST(AST*);
    void accept(ASTVisitor*);
    int size();
    std::string name(int);
};

class CallFunctionAST : public AST {
public:
    CallFunctionAST(string, AST*);
    void accept(ASTVisitor*);
    std::string name();
    ArgumentsAST* arguments();
};

class IfAST : public AST {
public:
    IfAST(AST*, AST*);
    IfAST(AST*, AST*, AST*);
    virtual void print(ostream&) const;
    AST* condition() const;
    AST* thenBlock() const;
    AST* elseBlock() const;
    void accept(ASTVisitor*);
};

class DefAST : public AST {
public:
    DefAST(string, AST*);
    DefAST(string, AST*, AST*);
    virtual void print(ostream&) const;
    std::string name() const;
    ArgumentsAST* arguments() const;
    AST* body() const;
    void accept(ASTVisitor*);
};

class TopAST : public AST {
public:
    void accept(ASTVisitor*);
};

class BlockAST : public AST {
public:
    BlockAST(AST*);
    void accept(ASTVisitor*);
};
