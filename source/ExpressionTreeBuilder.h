#ifndef SOURCE_EXPRESSIONTREEBUILDER_H_
#define SOURCE_EXPRESSIONTREEBUILDER_H_

#include <stack>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include "OperationNode.h"
#include "token.h"


//enum class TOKEN_TYPES {STRING, NUMBER, WORD, OPERATOR};


/****************************************************************************************
 *
 ****************************************************************************************/
struct PostfixError {
	PostfixError(std::string imsg) { msg = imsg; };
	std::string msg;
};


/****************************************************************************************
 *
 ****************************************************************************************/
class ExpressionTreeBuilder {

    private:
        static std::map<std::string, short> opHierarchy;
        std::stack<Token>          operators;
        std::stack<OperationNode*> operands;

    public:
        ExpressionTreeBuilder();
        ~ExpressionTreeBuilder();
        OperationNode* getExpressionTree(std::queue<Token> &toks) throw(PostfixError);
        static short getOperatorHeirchy(std::string op);
        static bool isPostUnary(std::string);
        static bool isPreUnary(std::string);
        static bool isControlWord(std::string op);

    private:
        void addOperation(bool isUnary);
        void makeParameter();
        void chainParameter();
        void addFunctionCall();
        void validateStatement(std::queue<Token> toks) throw (PostfixError);
        void static initializeHierarchy();

};


#endif
