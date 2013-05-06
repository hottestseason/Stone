#include <iostream>
#include "token.h"

int Token::getInteger() {
    throw "not integer token";
}

double Token::getDouble() {
    throw "not double token";
}

std::string Token::getText() {
    throw "not identifier token";
}

bool Token::isInteger() {
    return false;
}

bool Token::isDouble() {
    return false;
}

bool Token::isIdentifier() {
    return false;
}

IntegerToken::IntegerToken(int integer) {
    this->integer = integer;
}

int IntegerToken::getInteger() {
    return integer;
}

bool IntegerToken::isInteger() {
    return true;
}

DoubleToken::DoubleToken(double _double) {
    this->_double = _double;
}

double DoubleToken::getDouble() {
    return _double;
}

bool DoubleToken::isDouble() {
    return true;
}

IdentifierToken::IdentifierToken(std::string text) {
    this->text = text;
}

std::string IdentifierToken::getText() {
    return text;
}

bool IdentifierToken::isIdentifier() {
    return true;
}
