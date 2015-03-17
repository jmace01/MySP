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
        std::map<std::string, Variable**> variables;
        std::map<std::string, ClassDefinition* >* classes;
        static std::map<std::string, void (Executor::*)(void)> operationMap;
        Variable* returnVariable;
        OperationNode* currentNode;
        int lastValue;
        bool executeLeft;
        bool ternaryLeft;

    public:
        Executor();
        ~Executor();
        void run(std::map<std::string, ClassDefinition* >* classes);
        void preserveClasses(bool preserve);

    private:
        void initializeOperationMap();
        void executeInstruction(OperationNode* op) throw (RuntimeError);
        void executeOperator(OperationNode* op);
        void clearRegisters();
        void displayError(RuntimeError &e);

        //Operators
        void print();
        void assignment();
        void variableEquals();
        void typeEquals();
        void equals();
        void notVariableEquals();
        void notTypeEquals();
        void notEquals();
        void lessThan();
        void lessThanEqual();
        void greaterThan();
        void greaterThanEqual();
        void andd();
        void orr();
        void add();
        void sub();
        void mul();
        void div();
        void mod();
        void pow();
        void cat();
        void inc();
        void dec();
        void negate();
        void call();
        void iff();
        void jmp();
        void ternary();
        void staticVar();
        void dynamicVar();
        void arrayIndex();
};

#endif
