#ifndef SOURCE_TOKENIZER_H_
#define SOURCE_TOKENIZER_H_

#include "token.h"
#include <iostream>

class Tokenizer {
    private:
        unsigned int lineNumber;
        std::string infix;
        unsigned int infixPos;

        std::istream* input;

    public:
        Tokenizer();
        ~Tokenizer();
        void getTokens(std::string infix, std::queue<Token> &result);
        void getTokens(std::istream* stream, std::queue<Token> &result);

    private:
        void eatWhitespace(int &i);
        void eatComments(int &i);
        Token getNext(bool &wasOp);
        void eatWhitespace(char &currentChar);
        void eatComments(char &currentChar);
        Token getNextToken(bool &wasOp, char &currentChar);
};

#endif
