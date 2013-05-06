#pragma once

#include <string>

class Token {
public:
    virtual int getInteger();
    virtual double getDouble();
    virtual std::string getText();
    virtual bool isInteger();
    virtual bool isDouble();
    virtual bool isIdentifier();
};

class IntegerToken : public Token {
public:
    IntegerToken(int);
    int getInteger();
    bool isInteger();
private:
    int integer;
};

class DoubleToken : public Token {
public:
    DoubleToken(double);
    double getDouble();
    bool isDouble();
private:
    double _double;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(std::string);
    std::string getText();
    bool isIdentifier();
private:
    std::string text;
};
