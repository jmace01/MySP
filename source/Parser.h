#ifndef SOURCE_PARSER_H_
#define SOURCE_PARSER_H_

#include <map>
#include <string>
#include <queue>
#include <vector>
#include "Variables/Variable.h"
#include "ClassDefinition.h"
#include "ExpressionTreeBuilder.h"
#include "OperationNode.h"


/****************************************************************************************
 *
 ****************************************************************************************/
class Parser {
    private:
        std::queue<Token> toks;

        //Classes and methods
        std::map<std::string, ClassDefinition*>* classes;
        std::map<std::string, std::string> inheritance;
        ClassDefinition* currentClass;
        Method* currentMethod;

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
        std::map< std::string, ClassDefinition* >* parseText(std::string infix);
        void getTokens(std::string infix, std::queue<Token> &result);
        std::queue<PostfixError> getErrors();
        void clearErrors();

    private:
        void eatWhitespace(int &i);
        void eatComments(int &i);
        Token getNext();
        void addToken(Token &t, bool pushOnFunction);
        void addCondition(bool toFunction);
        void addMethodParameters();
        void buildTree();
        void startMain();
        void startClass(std::string &name);
        void startMethod(std::string &name, Visibility visibility, bool isStatic);
        void startProperty(Token &t);
        void endClass();
        void endMethod();
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
