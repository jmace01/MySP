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
    this->variables = map<string, Variable**>();
    this->returnVariable = NULL;
    this->currentNode = NULL;
    this->lastValue = 0;
    this->executeLeft = true;
    this->ternaryLeft = false;
    if (Executor::operationMap.empty()) {
        this->initializeOperationMap();
    }
}

map<string, void (Executor::*)(void)> Executor::operationMap = map<string, void (Executor::*)(void)>();


/****************************************************************************************
 *
 ****************************************************************************************/
Executor::~Executor() {
    //Remove classes if set
    if (this->deleteClasses) {
        map<string, ClassDefinition* >::iterator it;
        for (it = this->classes->begin(); it != this->classes->end(); it++) {
            delete it->second;
        }
    }
    //Remove variables
    map<string, Variable**>::iterator it;
    for (it = this->variables.begin(); it != this->variables.end(); it++) {
        delete (*it->second);
        delete it->second;
    }
    //Remove register data
    this->clearRegisters();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::initializeOperationMap() {
    operationMap["print"]  = &Executor::print;
    operationMap["echo"]   = &Executor::print;
    //operationMap["return"] = &Executor::print;
    //operationMap["break"]  = &Executor::print;
    operationMap["="]      = &Executor::assignment;
    //operationMap["+="]     = &Executor::print;
    //operationMap["-="]     = &Executor::print;
    //operationMap["*="]     = &Executor::print;
    //operationMap["/="]     = &Executor::print;
    //operationMap["%="]     = &Executor::print;
    //operationMap["^="]     = &Executor::print;
    operationMap["===="]   = &Executor::variableEquals;
    operationMap["==="]    = &Executor::typeEquals;
    operationMap["=="]     = &Executor::equals;
    operationMap["!==="]   = &Executor::notVariableEquals;
    operationMap["!=="]    = &Executor::notTypeEquals;
    operationMap["!="]     = &Executor::notEquals;
    operationMap["<"]      = &Executor::lessThan;
    operationMap["<="]     = &Executor::lessThanEqual;
    operationMap[">"]      = &Executor::greaterThan;
    operationMap[">="]     = &Executor::greaterThanEqual;
    operationMap["&&"]     = &Executor::andd;
    operationMap["||"]     = &Executor::orr;
    operationMap["."]      = &Executor::cat;
    operationMap["-"]      = &Executor::sub;
    operationMap["+"]      = &Executor::add;
    operationMap["*"]      = &Executor::mul;
    operationMap["/"]      = &Executor::div;
    operationMap["%"]      = &Executor::mod;
    operationMap["^"]      = &Executor::pow;
    operationMap["++"]     = &Executor::inc;
    operationMap["--"]     = &Executor::dec;
    operationMap["!"]      = &Executor::negate;
    //operationMap["~"]      = &Executor::print;
    //operationMap["&"]      = &Executor::print;
    operationMap["->"]     = &Executor::dynamicVar;
    operationMap["::"]     = &Executor::staticVar;
    //operationMap["P"]      = &Executor::print;
    operationMap["C"]      = &Executor::call;
    operationMap["jmp"]    = &Executor::jmp;
    operationMap["if"]     = &Executor::iff;
    operationMap["["]      = &Executor::arrayIndex;
    operationMap["?"]      = &Executor::ternary;
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

        try {
            executeInstruction(this->currentMethod->getInstruction(this->instructionPointer++));
            //For conditional statements
            if (!this->registerVariables.empty()) {
                this->lastValue = this->registerVariables.top()->getNumberValue();
                this->clearRegisters();
            }
        } catch (RuntimeError &e) {
            this->displayError(e);
            this->clearRegisters();
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::executeInstruction(OperationNode* op) throw (RuntimeError) {
    Variable* var;
    Variable** vPointer;
    this->executeLeft = true;

    //Deal with leaf nodes (values)
    if (op->operation.type != 'o') {
        //Number
        if (op->operation.type == 'n') {
            var = new Number(TEMP, strtod(op->operation.word.c_str(), NULL));
        }
        //String
        else if (op->operation.type == 's') {
            var = new String(TEMP, op->operation.word);
        }
        //Variables or constants
        else if (op->operation.type == 'w') {
            //
            //Constants? Handle them here
            //

            //Create the variable if it does not yet exist
            if (this->variables.find(op->operation.word) == this->variables.end()) {
                var = new Variable(PUBLIC);
                vPointer = new Variable*;
                *vPointer = var;
                var->setPointer(vPointer);
                this->variables[op->operation.word] = vPointer;
            }
            //Put the variable in the "register" stack
            var = *(this->variables[op->operation.word]);
        }
        this->registerVariables.push(var);
        return;
    }

    //Execute terminating operations like && and ||
    if (op->operation.isTerminating) {

        //Don't execute go down :: and -> nodes
        bool dontDescend = op->operation.word == "::" || op->operation.word == "->";

        //Get right node
        if (op->right != NULL && !dontDescend && op->operation.word != "C") {
            this->executeInstruction(op->right);
        }

        //Execute conditional
        try {
            this->executeOperator(op);
        } catch (RuntimeError &e) {
            e.line = op->operation.line;
            throw e;
        }

        //Execute left if needed
        if (op->left != NULL && this->executeLeft && !dontDescend) {
            if (op->operation.word == "?") {
                if (this->ternaryLeft) {
                    this->executeInstruction(op->left->left);
                } else {
                    this->executeInstruction(op->left->right);
                }
            } else {
                this->executeInstruction(op->left);
            }
        }

    }

    //Normal operation-- traverse left, then right, then execute
    else {

        //Get left node
        if (op->left != NULL) {
            this->executeInstruction(op->left);
        }


        //Get right node
        if (op->right != NULL) {
            this->executeInstruction(op->right);
        }

        //Execute no terminating operations
        try {
            this->executeOperator(op);
        } catch (RuntimeError &e) {
            e.line = op->operation.line;
            throw e;
        }
    }

    return;
}


/****************************************************************************************
 *
 ****************************************************************************************/
inline void Executor::executeOperator(OperationNode* op) {
    this->currentNode = op;
    string w = op->operation.word;

    void (Executor::*func)(void);
    func = operationMap[w];

    if (operationMap[w] != NULL) {
        (this->*func)();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::clearRegisters() {
    Variable* v;
    while (!this->registerVariables.empty()) {
        v = this->registerVariables.top();
        if (v->getVisibility() == TEMP) {
            delete v;
        }
        this->registerVariables.pop();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::displayError(RuntimeError &e) {
    string header;
    if (e.level == WARNING) {
        header = "WARNING: ";
    } else if (e.level == ERROR) {
        header = "ERROR: ";
    } else {
        header = "FATAL ERROR: ";
    }
    cout << header << e.msg << " (line " << e.line << ')' << endl;
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
void Executor::print() {
    Variable* v = this->registerVariables.top();

    cout << v->getStringValue();

    if (v->getVisibility() == TEMP) {
        delete v;
    }

    this->registerVariables.pop();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::assignment() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    if (a->getVisibility() == TEMP) {
        throw RuntimeError("Assignment to value instead of variable", WARNING);
    }

    //Create the new variable
    if (b->getType() == 'n') {
        result = new Number(a->getVisibility(), b->getNumberValue());
    } else if (b->getType() == 's') {
        string s = b->getStringValue();
        result = new String(a->getVisibility(), s);
    } else if (b->getType() == 'a') {
        result = new Array(a->getVisibility());
    } else if (b->getType() == 'o') {
        result = new Object(a->getVisibility(), (Object*) b);
    } else {
        result = new Variable(a->getVisibility());
    }

    //Set the new variable
    result->setPointer(a->getPointer());
    *(a->getPointer()) = result;
    delete a;

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::variableEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            a == b
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::typeEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a == *b && a->getType() == b->getType()
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::equals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a == *b
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notVariableEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            a == b
        )
    {
        result = new Number(TEMP, 0);
    } else {
        result = new Number(TEMP, 1);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notTypeEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a == *b && a->getType() == b->getType()
        )
    {
        result = new Number(TEMP, 0);
    } else {
        result = new Number(TEMP, 1);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a == *b
        )
    {
        result = new Number(TEMP, 0);
    } else {
        result = new Number(TEMP, 1);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::lessThan() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a < *b
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::lessThanEqual() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            (*a < *b) || (*a == *b)
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::greaterThan() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            *a > *b
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::greaterThanEqual() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (
            (*a > *b) || (*a == *b)
        )
    {
        result = new Number(TEMP, 1);
    } else {
        result = new Number(TEMP, 0);
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::andd() {
    Variable* a;

    //Get operand values
    a = this->registerVariables.top();

    //Compute result
    this->executeLeft = (a->getBooleanValue());

    if (this->executeLeft) {
        //Delete operand a if visibility is TEMP
        this->registerVariables.pop();
        if (a->getVisibility() == TEMP) {
            delete a;
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::orr() {
    Variable* a;

    //Get operand values
    a = this->registerVariables.top();

    //Compute result
    this->executeLeft = (!a->getBooleanValue());

    if (this->executeLeft) {
        //Delete operand a if visibility is TEMP
        this->registerVariables.pop();
        if (a->getVisibility() == TEMP) {
            delete a;
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::add() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a + *b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::sub() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a - *b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::mul() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a * *b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::div() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a / *b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::mod() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a % *b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::pow() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = a->power(*b);

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::cat() {
    Variable* a;
    Variable* b;
    Variable* t;
    string s;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (a->getType() == 's') {
        result = a->concat(*b);
    } else {
        s = a->getStringValue();
        t = new String(TEMP, s);
        result = t->concat(*b);
        delete t;
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::inc() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a)++;

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::dec() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = (*a)--;

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::negate() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    result = new Number(TEMP, !a->getBooleanValue());

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::call() {
    OperationNode* nextOp;
    Variable* a;
    Variable* result;

    //Compute result
    this->executeLeft = false;
    nextOp = this->currentNode->right;

    bool isStatic = nextOp->operation.word == "::";
    bool isDynamic = nextOp->operation.word == "->";

    //Is this a method call?
    if (isStatic || isDynamic) {

        //Get the variable
        this->executeInstruction(this->currentNode->right->right);
        a = this->registerVariables.top();
        this->registerVariables.pop();

        //Call static function
        if (isStatic) {
            //cout << "STATIC FUNCTION CALL" << endl;
        } else {
            //cout << "DYNAMIC FUNCTION CALL" << endl;
        }

        //Get the result
        result = new Variable(TEMP);

    } else {

        //Create an array
        if (this->currentNode->right->operation.word == "array") {
            result = new Array(TEMP);
        }

        //Create a new class instance
        else {
            if ((*this->classes).find(this->currentNode->right->operation.word) == this->classes->end()) {
                throw RuntimeError("No class found", FATAL);
            }
            result = new Object(TEMP, (*this->classes)[this->currentNode->right->operation.word]);
        }

    }

    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::staticVar() {
    Variable* a;
    Variable* result;

    //Get the class name
    string className = this->currentNode->right->operation.word;

    //Compute result
    this->executeLeft = false;
    if ((*this->classes).find(className) == (*this->classes).end()) {
        throw RuntimeError("Unknown class '"+className+"'",ERROR);
    }
    ClassDefinition* cls = (*this->classes)[className];
    result = cls->getStaticProperty(this->currentNode->left->operation.word);

    if (result->getVisibility() != PUBLIC) {
        throw RuntimeError("Cannot access private variable from outside class",ERROR);
    }

    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::dynamicVar() {
    Variable* a;
    Variable* result;

    //Get the class
    this->executeInstruction(this->currentNode->right);
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    this->executeLeft = false;
    result = a->getProperty(this->currentNode->left->operation.word);

    if (result->getVisibility() != PUBLIC) {
        throw RuntimeError("Cannot access private variable from outside class",ERROR);
    }

    this->registerVariables.push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::iff() {
    Variable* a;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (!this->lastValue) { //If not true, take the jump
        this->instructionPointer = a->getNumberValue();
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::jmp() {
    Variable* a;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    this->instructionPointer = a->getNumberValue();

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::ternary() {
    Variable* a;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    this->ternaryLeft = !(a->getBooleanValue());

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::arrayIndex() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables.top();
    this->registerVariables.pop();
    b = this->registerVariables.top();
    this->registerVariables.pop();

    //Compute result
    if (a->getType() != 'a') {
        throw RuntimeError("Cannot index from type "+a->getTypeString(), WARNING);
    } else {
        result = a->getArrayValue(b->getStringValue());
    }

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables.push(result);
}
