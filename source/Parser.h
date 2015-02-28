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
        bool inFunction;


    public:
        Parser();
        ~Parser();
        void initKeywords();
        short static isKeyWord(std::string word);
        std::map< std::string, std::vector<OperationNode*> >* parseText(std::string infix);
        void getTokens(std::string infix, std::queue<Token> &result);
        std::queue<PostfixError> getErrors();
        void clearErrors();

    private:
        Token getNext();
        void addToken(Token &t, bool pushOnFunction);
        void addCondition(bool toFunction);
        void buildTree();
        void startFunction();
        void getStatement(Token &t, bool isFor);
        void addStatement(bool toFunction);
        void markScoped(OperationNode* op);
        void endScope(bool setFirst);
        void endWhile();
        void endDoWhile();
        void beginFor(Token &t, std::string &lowercaseWord);
        void endFor();
        OperationNode* createJump(unsigned long pos, bool includePos);
};

#endif
