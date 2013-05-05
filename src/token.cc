#include <iostream>
#include "token.h"

double Token::number() {
    throw "not number token";
}

string Token::text() {
    throw "not identifier token";
}

bool Token::isNumber() {
    return false;
}

bool Token::isIdentifier() {
    return false;
}

NumberToken::NumberToken(double number) {
    mNumber = number;
}

double NumberToken::number() {
    return mNumber;
}

bool NumberToken::isNumber() {
    return true;
}

IdentifierToken::IdentifierToken(string text) {
    mText = text;
}

string IdentifierToken::text() {
    return mText;
}

bool IdentifierToken::isIdentifier() {
    return true;
}
