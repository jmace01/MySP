#ifndef SOURCE_EXECUTOR_H_
#define SOURCE_EXECUTOR_H_


#include <map>
#include <stack>
#include <string>
#include "ClassDefinition.h"
#include "ExpressionTreeBuilder.h"
#include "Method.h"
#include "OperationNode.h"
#include "Variables/Array.h"
#include "Variables/Number.h"
#include "Variables/Object.h"
#include "Variables/String.h"
#include "Variables/Variable.h"


struct Scope {
        unsigned long instructionPointer;
        Method* method;
        std::map<std::string, Variable*> variables;
};


class Executor {
    private:
        bool deleteClasses;
        unsigned long instructionPointer;
        Method* currentMethod;
        std::stack<Scope> scopeStack;
        std::stack<Variable*> registerVariables;
        std::map<std::string, Variable*> variables;
        std::map<std::string, ClassDefinition* >* classes;
        static std::map<std::string, void (Executor::*)(void)> operationMap;
        Variable* returnVariable;

    public:
        Executor();
        ~Executor();
        void run(std::map<std::string, ClassDefinition* >* classes);
        void preserveClasses(bool preserve);

    private:
        void initializeOperationMap();
        void executeInstruction(OperationNode* op);
        void executeOperator(OperationNode* op);

        //Operators
        void add();
};

#endif
