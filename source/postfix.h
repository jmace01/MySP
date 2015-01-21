#ifndef SOURCE_POSTFIX_H_
#define SOURCE_POSTFIX_H_

#include <stack>
#include <string>
#include <vector>
#include <queue>


/******************************************************************
 *
 ******************************************************************/
struct Token{
    std::string word;
    char type;
    int line;
};


/******************************************************************
 *
 ******************************************************************/
class Postfix {

    private:
        int          infixPos;
        std::string  infix;
        unsigned int lineNumber;

    public:
        Postfix();
        ~Postfix();

        std::vector<Token> getPostfix(std::string infix, unsigned int lineNumber);

    private:
        std::queue<Token> getTokens();
        Token getNext();
        int getOperatorHeirchy(std::string op);
        bool isPostUnary(std::string);
        bool isPreUnary(std::string);

};


#endif
