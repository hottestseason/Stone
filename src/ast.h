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
    virtual void print(std::ostream&) const;
    virtual void accept(ASTVisitor*) = 0;
    friend std::ostream& operator<<(std::ostream&, const AST&);
private:
    std::vector<AST*>* mChildren;
};

class ASTLeaf : public AST {
public:
    ASTLeaf(Token*);
    Token* token() const;
    void print(std::ostream&) const;
    void accept(ASTVisitor*);
private:
    Token* mToken;
};

class ValuableAST : public AST {
public:
    ValuableAST(std::string);
    ValuableAST(std::string, std::string);
    void accept(ASTVisitor*);
    std::string getName();
    std::string getTypeName();
};

class BinaryExprAST : public AST {
public:
    BinaryExprAST(std::string, AST*);
    BinaryExprAST(std::string, AST*, AST*);
    std::string op();
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
    ValuableAST *get(int);
};

class CallFunctionAST : public AST {
public:
    CallFunctionAST(std::string, AST*);
    void accept(ASTVisitor*);
    std::string name();
    ArgumentsAST* arguments();
};

class IfAST : public AST {
public:
    IfAST(AST*, AST*);
    IfAST(AST*, AST*, AST*);
    virtual void print(std::ostream&) const;
    AST* condition() const;
    AST* thenBlock() const;
    AST* elseBlock() const;
    void accept(ASTVisitor*);
};

class DefAST : public AST {
public:
    DefAST(std::string, AST*, std::string);
    DefAST(std::string, AST*, AST*, std::string);
    virtual void print(std::ostream&) const;
    std::string name() const;
    ArgumentsAST* arguments() const;
    AST* body() const;
    std::string getTypeName();
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
