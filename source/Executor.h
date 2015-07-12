#ifndef SOURCE_EXECUTOR_H_
#define SOURCE_EXECUTOR_H_

#include <iostream>
#include <map>
#include <string>
#include <stack>
#include "ClassDefinition.h"
#include "Method.h"
#include "Variables/Array.h"
#include "Variables/Nil.h"
#include "Variables/Number.h"
#include "Variables/Object.h"
#include "Variables/String.h"
#include "Variables/Variable.h"


struct Scope {
    //
};


class Executor {
    public:
        static std::map<std::string, Variable*> constants;

    private:
        Method* currentMethod;
        long currentInstruction;
        std::stack<Scope> scopeStack;
        std::vector<Instruction> methodInstructions;

    public:
        Executor();
        ~Executor();

        void run(std::map<std::string, ClassDefinition* >* classes);

        void print();
        void ret();
        void brk();
        void cont();
        void thrw();
        void assignment();
        void addAssign();
        void subAssign();
        void multAssign();
        void divAssign();
        void modAssign();
        void powAssign();
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
        void cat();
        void sub();
        void add();
        void mul();
        void div();
        void pow();
        void mod();
        void inc();
        void dec();
        void negate();
        void reference();
        void deletion();
        void dynamicVar();
        void staticVar();
        void parameter();
        void call();
        void jmp();
        void jmpNotTrue();
        void jmpTrue();
        void arrayIndex();
        void tryBlock();
        void catchBlock();
        void finallyBlock();

        static Variable* makeVariableCopy(Variable* var, Visibility vis);
};

#endif
