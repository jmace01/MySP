#include "Executor.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Executor::Executor() {
    this->instructionPointer = 0;
    this->deleteClasses = true;
    this->currentMethod = NULL;
    this->classes = NULL;
    this->scopeStack = stack<Scope>();
    this->registerVariables = stack<Variable*>();
    this->variables = map<string, Variable*>();
    this->returnVariable = NULL;
    if (Executor::operationMap.empty()) {
        this->initializeOperationMap();
    }
}

map<string, void (Executor::*)(void)> Executor::operationMap = map<string, void (Executor::*)(void)>();

/****************************************************************************************
 *
 ****************************************************************************************/
Executor::~Executor() {
    if (this->deleteClasses) {
        map<string, ClassDefinition* >::iterator it;
        for (it = this->classes->begin(); it != this->classes->end(); it++) {
            delete it->second;
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::initializeOperationMap() {
    operationMap["print"]  = &Executor::add;
    operationMap["echo"]   = &Executor::add;
    operationMap["return"] = &Executor::add;
    operationMap["break"]  = &Executor::add;
    operationMap["="]      = &Executor::add;
    operationMap["+="]     = &Executor::add;
    operationMap["-="]     = &Executor::add;
    operationMap["*="]     = &Executor::add;
    operationMap["/="]     = &Executor::add;
    operationMap["%"]      = &Executor::add;
    operationMap["^="]     = &Executor::add;
    operationMap["===="]   = &Executor::add;
    operationMap["==="]    = &Executor::add;
    operationMap["=="]     = &Executor::add;
    operationMap["!==="]   = &Executor::add;
    operationMap["!=="]    = &Executor::add;
    operationMap["!="]     = &Executor::add;
    operationMap["<"]      = &Executor::add;
    operationMap["<="]     = &Executor::add;
    operationMap[">"]      = &Executor::add;
    operationMap[">="]     = &Executor::add;
    operationMap["&&"]     = &Executor::add;
    operationMap["||"]     = &Executor::add;
    operationMap["."]      = &Executor::add;
    operationMap["-"]      = &Executor::add;
    operationMap["+"]      = &Executor::add;
    operationMap["*"]      = &Executor::add;
    operationMap["/"]      = &Executor::add;
    operationMap["%"]      = &Executor::add;
    operationMap["^"]      = &Executor::add;
    operationMap["++"]     = &Executor::add;
    operationMap["--"]     = &Executor::add;
    operationMap["!"]      = &Executor::add;
    operationMap["~"]      = &Executor::add;
    operationMap["&"]      = &Executor::add;
    operationMap["->"]     = &Executor::add;
    operationMap["::"]     = &Executor::add;
    operationMap["P"]      = &Executor::add;
    operationMap["C"]      = &Executor::add;
    operationMap[""]       = &Executor::add;
    operationMap[""]       = &Executor::add;
    operationMap[""]       = &Executor::add;
    operationMap[""]       = &Executor::add;
}

/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::run(map<string, ClassDefinition* >* classes) {
    this->instructionPointer = 0;
    this->classes = classes;

    string startMethod = "main";
    this->currentMethod = (*this->classes)["~"]->getMethod(startMethod);
    this->scopeStack.push(Scope());

    while (!scopeStack.empty()) {
        if (instructionPointer >= this->currentMethod->getInstructionSize()) {
            this->scopeStack.pop();
            continue;
        }
        executeInstruction(this->currentMethod->getInstruction(this->instructionPointer++));
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::executeInstruction(OperationNode* op) {
    Variable* var;

    //Deal with leaf nodes (values)
    if (op->operation.type != 'o') {
        //Number
        if (op->operation.type == 'n') {
            var = new Number(PUBLIC, false, strtod(op->operation.word.c_str(), NULL));
        }
        //String
        else if (op->operation.type == 's') {
            var = new String(PUBLIC, false, op->operation.word);
        }
        //Variables or constants
        else if (op->operation.type == 'w') {
            //Constants? Handle them here
            //

            //Create the variable if it does not yet exist
            if (this->variables.find(op->operation.word) == this->variables.end()) {
                this->variables[op->operation.word] = new Variable(PUBLIC, false);
            }
            //Put the variable in the "register" stack
            var = this->variables[op->operation.word];
        }
        this->registerVariables.push(var);
        return;
    }

    //Jump down to leaf nodes to start
    if (op->left != NULL) {
        this->executeInstruction(op->left);
    }
    if (op->right != NULL) {
        this->executeInstruction(op->right);
    }

    //Execute operation
    this->executeOperator(op);

    return;
}


/****************************************************************************************
 *
 ****************************************************************************************/
inline void Executor::executeOperator(OperationNode* op) {
    string w = op->operation.word;

    void (Executor::*func)(void);
    func = operationMap[w];

    if (operationMap[w] != NULL) {
        (this->*func)();
    }
}


/****************************************************************************************
 * Should the class definitions be freed when the destructor runs?
 ****************************************************************************************/
void Executor::preserveClasses(bool preserve) {
    this->deleteClasses = !preserve;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::add() {
    cout << "YES!" << endl;
}
