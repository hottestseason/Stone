#pragma once

#include <string>
using namespace std;

class Token {
public:
    virtual double number();
    virtual string text();
    virtual bool isNumber();
    virtual bool isIdentifier();
};

class NumberToken : public Token {
public:
    NumberToken(double);
    double number();
    bool isNumber();
private:
    double mNumber;
};

class IdentifierToken : public Token {
public:
    IdentifierToken(string);
    string text();
    bool isIdentifier();
private:
    string mText;
};
