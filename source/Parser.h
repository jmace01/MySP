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
        bool isMain;

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
        std::map< std::string, ClassDefinition* >* parseTokens(std::queue<Token> &intoks);
        std::queue<PostfixError> getErrors();
        void clearErrors();

    private:
        void addToken(Token &t, bool pushOnFunction);
        void addCondition(bool toFunction, Token t);
        void addMethodParameters(Token &t);
        void buildTree();
        void startMain(Token &t);
        void startClass(std::string &name, Token &t);
        void startMethod(std::string &name, Visibility visibility, bool isStatic, Token &t);
        void startProperty(Token &t);
        void endClass();
        void endMethod();
        void getStatement(Token &t, bool isFor);
        void addStatement(bool toFunction, Token t);
        void markScoped(OperationNode* op);
        void endScope(bool setFirst);
        void endWhile();
        void endDoWhile();
        void beginFor(Token &t, std::string &lowercaseWord);
        void endFor();
        void endTry();
        void endFinally();
        void endCatch();
        OperationNode* createJump(unsigned long pos, bool includePos);
};

#endif
