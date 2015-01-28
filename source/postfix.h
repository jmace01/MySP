#ifndef SOURCE_POSTFIX_H_
#define SOURCE_POSTFIX_H_

#include <stack>
#include <string>
#include <vector>
#include <queue>


//enum class TOKEN_TYPES {STRING, NUMBER, WORD, OPERATOR};


/******************************************************************
 *
 ******************************************************************/
struct PostfixError {
	PostfixError(std::string imsg) { msg = imsg; };
	std::string msg;
};


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
        int                infixPos;
        std::string        infix;
        unsigned int       lineNumber;
        std::string        tempVariableNumber;
        std::stack<Token>  operators;
        std::stack<Token>  operands;
        std::vector<Token> result;

    public:
        Postfix();
        ~Postfix();

        std::vector<Token> getPostfix(std::string infix, unsigned int lineNumber) throw(PostfixError);

    private:
        std::queue<Token> getTokens();
        Token getNext();
        int getOperatorHeirchy(std::string op);
        bool isPostUnary(std::string);
        bool isPreUnary(std::string);
        bool isControlWord(std::string op);
        void addTemporary();
        void addOperator(std::string op);
        void addTopOperand();
        void addTopOperator();
        void validateStatement(std::queue<Token> toks) throw (PostfixError);

};


#endif
