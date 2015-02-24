#ifndef SOURCE_PARSER_H_
#define SOURCE_PARSER_H_

#include <map>
#include <string>
#include <queue>
#include <vector>
#include "ExpressionTreeBuilder.h"
#include "OperationNode.h"


/****************************************************************************************
 *
 ****************************************************************************************/
class Parser {
    private:
        std::queue<Token> toks;
        std::map< std::string, std::vector<OperationNode*> >* functions;
        std::string currentFunction;
        static std::map<std::string, short> keywords;
        ExpressionTreeBuilder expTreeBuilder;
        std::queue<Token> statementQueue;
        std::stack<OperationNode*> controlStack;
        unsigned int lineNumber;
        std::string infix;
        unsigned int infixPos;
        std::queue<PostfixError> errors;
        bool upcomingElse;


    public:
        Parser();
        ~Parser();
        void initKeywords();
        short static isKeyWord(std::string word);
        std::map< std::string, std::vector<OperationNode*> >* parseText(std::string infix);
        void getTokens(std::string infix, std::queue<Token> &result);

    private:
        Token getNext();
        void addCondition();
        void buildTree();
        void addStatement();
        void markScoped(OperationNode* op);
        void endScope(bool setFirst);
};

#endif
