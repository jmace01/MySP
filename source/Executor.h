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
#include "Variables/Nil.h"
#include "Variables/Number.h"
#include "Variables/Object.h"
#include "Variables/String.h"
#include "Variables/Variable.h"


//Used to throw in order to break out of a recursive traversal of an expression tree
struct FunctionCall{};


//Used to hold scope information
struct Scope {
        unsigned long instructionPointer;
        Method* method;
        Variable* currentObject;
        ClassDefinition* currentClass;
        std::map<std::string, Variable**>* variables;
        std::stack<Variable*>* registerVariables;
        OperationNode* currentNode;
        bool isConstructor;
};


class Executor {
    public:
        static std::map<std::string, Variable*> constants;

    private:
        bool deleteClasses;
        unsigned long instructionPointer;
        Method* currentMethod;
        Variable* currentObject;
        ClassDefinition* currentClass;
        std::stack<Scope> scopeStack;
        std::stack<Variable*>* registerVariables;
        std::stack<Variable*> parameterStack;
        std::map<std::string, Variable**>* variables;
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
        static Variable* makeVariableCopy(Variable* v, Visibility visibility);

    private:
        void initializeOperationMap();
        void initializeConstants();
        void executeInstruction(OperationNode* op, std::string recover) throw (RuntimeError, FunctionCall);
        void loadValue(OperationNode* op);
        void executeOperator(OperationNode* op);
        std::string recoverPosition(OperationNode* op, char direction);
        void clearRegisters();
        void displayError(RuntimeError &e);
        void initMethodCall(Method* method, Variable* object, ClassDefinition* classDef, bool isStatic, bool isConstructor, std::string &methodName);

        //Operators
        void print();
        void ret();
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
        void parameter();
        void loadMethodParameters();
        void call();
        void iff();
        void jmp();
        void ternary();
        void staticVar();
        void dynamicVar();
        void arrayIndex();
};

#endif
