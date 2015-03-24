#include "Executor.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 * Executor Constructor
 ****************************************************************************************/
Executor::Executor() {
    //Execution and clean up
    this->instructionPointer = 0;   //What instruction are we executing?
    this->deleteClasses = true;     //Should classes be deleted when destructor is called?
    this->currentMethod = NULL;     //The current method being executed
    this->currentObject = NULL;     //Saves object for "this" variable
    this->classes = NULL;           //The class definitions from the Parser

    //Scope variables
    this->scopeStack = stack<Scope>();                //The scope for function calls
    this->registerVariables = new stack<Variable*>(); //The temporary values for expressions
    this->parameterStack = stack<Variable*>();        //Parameters for method calls
    this->variables = new map<string, Variable**>();  //The variables to be stored

    this->initializeConstants();

    //Special case variables (Used in special circumstance operations)
    this->returnVariable = NULL;    //The return value of a function
    this->currentNode = NULL;       //The current node being executed
    this->lastValue = 0;            //Ending value of last statement (used in conditions)
    this->executeLeft = true;       //In a short circuit expression, should the other side be executed?
    this->ternaryLeft = false;      //In a ternary statement, which side should be executed?

    //The operation map routes operations (such as "+") to method (such as Executer::add)
    //The first time an Executor class is initialized, the map should be populated
    if (Executor::operationMap.empty()) {
        this->initializeOperationMap();
    }
}


//Create the operation-to-method map
map<string, void (Executor::*)(void)> Executor::operationMap = map<string, void (Executor::*)(void)>();

//Create constants map
map<string, Variable*> Executor::constants = map<string, Variable*>();


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
    for (it = this->variables->begin(); it != this->variables->end(); it++) {
        delete (*it->second);
        delete it->second;
    }
    //Remove constants
    map<string, Variable*>::iterator cit;
    for (cit = this->constants.begin(); cit != this->constants.end(); cit++) {
        delete cit->second;
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
    operationMap["return"] = &Executor::ret;
    //operationMap["break"]  = &Executor::print;
    //operationMap["continue"]  = &Executor::print;
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
    operationMap["P"]      = &Executor::parameter;
    operationMap["C"]      = &Executor::call;
    operationMap["jmp"]    = &Executor::jmp;
    operationMap["if"]     = &Executor::iff;
    operationMap["["]      = &Executor::arrayIndex;
    operationMap["?"]      = &Executor::ternary;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::initializeConstants() {
    this->constants["false"]          = new Number(CONST, 0);
    this->constants["true"]           = new Number(CONST, 1);
    this->constants["null"]           = new Nil(CONST);
    this->constants["pi"]             = new Number(CONST, 3.14159);
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

    string recover = "";

    while (!scopeStack.empty()) {
        //If the method end has been reached, pop off the method and get the previous one
        if (instructionPointer >= this->currentMethod->getInstructionSize()) {
            if (this->scopeStack.size() > 1) {
                Scope scope = this->scopeStack.top();
                this->instructionPointer = scope.instructionPointer;
                this->variables = scope.variables;
                this->registerVariables = scope.registerVariables;
                this->currentMethod = scope.method;
                this->currentObject = scope.currentObject;
                this->currentNode = scope.currentNode;
                recover = this->recoverPosition(this->currentMethod->getInstruction(this->instructionPointer), '0');
                //Does the line execution need to be finished?
                if (recover != "" && recover != "0") {
                    //Put the return value in the register
                    if (this->returnVariable != NULL) {
                        this->registerVariables->push(this->returnVariable);
                    }
                    //Default return value
                    else {
                        this->registerVariables->push(new Nil(TEMP));
                    }
                }
                //The last line was finished, move on
                else {
                    if (this->returnVariable != NULL) {
                        delete this->returnVariable;
                    }
                    this->instructionPointer++;
                }
            }
            this->returnVariable = NULL;
            this->scopeStack.pop();
            continue;
        }

        try {
            //Execute instruction at instruction pointer
            executeInstruction(this->currentMethod->getInstruction(this->instructionPointer++), recover);
            //For conditional statements, save the last value
            if (!this->registerVariables->empty()) {
                this->lastValue = this->registerVariables->top()->getNumberValue();
                this->clearRegisters();
            }
        } catch (RuntimeError &e) {
            this->displayError(e);
            this->clearRegisters();
        } catch (FunctionCall &e) {}

        recover = "";
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::loadValue(OperationNode * op) {
    Variable* var;
    Variable** vPointer;

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
        var = this->constants[op->operation.word];
        if (var != NULL) {
            this->registerVariables->push(var);
            return;
        }

        //Create the variable if it does not yet exist
        if (this->variables->find(op->operation.word) == this->variables->end()) {
            var = new Variable(PUBLIC);
            vPointer = new Variable*;
            *vPointer = var;
            var->setPointer(vPointer);
            (*this->variables)[op->operation.word] = vPointer;
        }
        //Put the variable in the "register" stack
        var = *((*this->variables)[op->operation.word]);
    }
    this->registerVariables->push(var);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::executeInstruction(OperationNode* op, string recover) throw (RuntimeError, FunctionCall) {
    this->executeLeft = true;
    bool recoverLeft  = false;
    bool recoverRight = false;

    //We left off before for a method call. Go back to where we were.
    if (recover != "") {
        if (recover == "0") {
            return; //We just returned from our function call
        } else if (recover[1] == 'L') {
            recoverLeft = true;
            this->executeInstruction(op->left, "0"+recover.substr(2));
        } else {
            recoverLeft  = true;
            recoverRight = true;
            this->executeInstruction(op->right, "0"+recover.substr(2));
        }
    }

    //Deal with leaf nodes (values)
    if (op->operation.type != 'o') {
        this->loadValue(op);
        return;
    }

    //Execute terminating operations like && and ||
    if (op->operation.isTerminating) {
        //Ignore : on recover
        if (op->operation.word == ":") return;

        //Ignore ? if its already been executed
        if (recoverLeft && !recoverRight && op->operation.word == "?") {
            return;
        }

        //Don't execute go down :: and -> nodes
        bool dontDescend = op->operation.word == "::" || op->operation.word == "->";

        //Get right node unless its a function call, then go left
        if (op->right != NULL && !dontDescend && op->operation.word != "C" && !recoverRight) {
            this->executeInstruction(op->right, recover);
        } else if (op->left != NULL && op->operation.word == "C" && !recoverRight) {
            this->executeInstruction(op->left, recover);
        }

        //Execute conditional
        //Avoid executing it twice by checking if on side was already executed
        if (!recoverLeft || recoverRight) {
            try {
                this->executeOperator(op);
            } catch (RuntimeError &e) {
                e.line = op->operation.line;
                throw e;
            }
        }

        //Execute left if needed
        if (op->left != NULL && this->executeLeft && !dontDescend && op->operation.word != "C" && (recoverRight || !recoverLeft)) {
            if (op->operation.word == "?") {
                if (this->ternaryLeft) {
                    this->executeInstruction(op->left->left, recover);
                } else {
                    this->executeInstruction(op->left->right, recover);
                }
            } else {
                this->executeInstruction(op->left, "");
            }
        }
    }

    //Normal operation-- traverse left, then right, then execute
    else {
        //Get left node
        if (op->left != NULL && !recoverLeft) {
            this->executeInstruction(op->left, recover);
        }


        //Get right node
        if (op->right != NULL && !recoverRight) {
            this->executeInstruction(op->right, recover);
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
string Executor::recoverPosition(OperationNode* op, char direction) {
    string result = "";

    if (op == NULL) {
        return result;
    } else if (op == this->currentNode) {
        result += direction;
    } else {
        result = recoverPosition(op->left, 'L');
        if (result != "") {
            return direction + result;
        } else {
            result = recoverPosition(op->right, 'R');
            if (result != "") {
                return direction + result;
            } else {
                return "";
            }
        }
    }

    return result;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::clearRegisters() {
    Variable* v;
    while (!this->registerVariables->empty()) {
        v = this->registerVariables->top();
        if (v->getVisibility() == TEMP) {
            delete v;
        }
        this->registerVariables->pop();
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
    Variable* v = this->registerVariables->top();

    cout << v->getStringValue();

    if (v->getVisibility() == TEMP) {
        delete v;
    }

    this->registerVariables->pop();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::ret() {
    Variable* v = NULL;

    //Is there a return value?
    if (this->registerVariables->size() > 0) {
        v = this->registerVariables->top();
        this->registerVariables->pop();
    }

    this->returnVariable = v;
    this->instructionPointer = this->currentMethod->getInstructionSize() + 1;
}


/****************************************************************************************
 * Creates a copy of an input variable
 ****************************************************************************************/
Variable* Executor::makeVariableCopy(Variable* v, Visibility visibility) {
    Variable* result;

    if (v->getType() == 'n') {
        result = new Number(visibility, v->getNumberValue());
    } else if (v->getType() == 's') {
        string s = v->getStringValue();
        result = new String(visibility, s);
    } else if (v->getType() == 'a') {
        result = new Array(visibility);
    } else if (v->getType() == 'o') {
        result = new Object(visibility, (Object*) v);
    } else if (v->getType() == '0') {
            result = new Nil(visibility);
    } else {
        result = new Variable(visibility);
    }

    return result;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::assignment() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

    if (a->getVisibility() == TEMP || a->getVisibility() == CONST) {
        throw RuntimeError("Assignment to value instead of variable", WARNING);
    }

    //Create the new variable
    result = Executor::makeVariableCopy(b, a->getVisibility());

    //Set the new variable
    result->setPointer(a->getPointer()); //Set pointer in map
    *(result->getPointer()) = result; //Set the point back to map
    delete a;

    //Delete operand b if visibility is TEMP
    if (b->getVisibility() == TEMP) {
        delete b;
    }

    //Push on result
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::variableEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::typeEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::equals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notVariableEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notTypeEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::notEquals() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::lessThan() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::lessThanEqual() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::greaterThan() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::greaterThanEqual() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::andd() {
    Variable* a;

    //Get operand values
    a = this->registerVariables->top();

    //Compute result
    this->executeLeft = (a->getBooleanValue());

    if (this->executeLeft) {
        //Delete operand a if visibility is TEMP
        this->registerVariables->pop();
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
    a = this->registerVariables->top();

    //Compute result
    this->executeLeft = (!a->getBooleanValue());

    if (this->executeLeft) {
        //Delete operand a if visibility is TEMP
        this->registerVariables->pop();
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
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::sub() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::mul() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::div() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::mod() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::pow() {
    Variable* a;
    Variable* b;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
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
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::inc() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

    if (a->getVisibility() == CONST) {
        throw RuntimeError("Cannot increment a constant", ERROR);
    }

    //Compute result
    result = (*a)++;

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::dec() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

    if (a->getVisibility() == CONST) {
        throw RuntimeError("Cannot increment a constant", ERROR);
    }

    //Compute result
    result = (*a)--;

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::negate() {
    Variable* a;
    Variable* result;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

    //Compute result
    result = new Number(TEMP, !a->getBooleanValue());

    //Delete operand a if visibility is TEMP
    if (a->getVisibility() == TEMP) {
        delete a;
    }

    //Push on result
    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::parameter() {
    Variable* a;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

    this->parameterStack.push(a);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::loadMethodParameters() {
    OperationNode* node = this->currentNode->left;

    int i; //How many parameters do we need have?
    //See how many "P" nodes are left of the call
    for(i = 0; node; i++, node = node->left);

    //Are there enough parameters?
    if (i < this->currentMethod->getMinParameters()) {
        throw RuntimeError("Not enough parameters", FATAL);
    }

    //Are there too many parameters?
    else if (i > this->currentMethod->getMaxParameters()) {
        throw RuntimeError("Too many parameters", FATAL);
    }

    //Put parameters in scope
    string pName;
    Variable* value;
    Variable** vPointer;

    for (int j = 0; j < this->currentMethod->getParameterSize(); j++) {
        //Get the parameter name
        pName = this->currentMethod->getParameter(j);
        //Is the parameter value on the stack?
        if (j < i) {
            value = this->parameterStack.top();
            this->parameterStack.pop();
        }
        //If not, get the default value
        else {
            value = this->currentMethod->getDefaultParameter(j);
        }
        value = this->makeVariableCopy(value, PUBLIC);
        vPointer = new Variable*;
        *vPointer = value;
        value->setPointer(vPointer);
        (*this->variables)[pName] = vPointer;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::call() {
    OperationNode* nextOp;
    Variable* a = NULL;
    Variable* result;

    //Compute result
    this->executeLeft = false;
    nextOp = this->currentNode->right;

    bool isStatic = nextOp->operation.word == "::";
    bool isDynamic = nextOp->operation.word == "->";

    //Is this a method call?
    if (isStatic || isDynamic) {
        Method* method;
        ClassDefinition* newClass;

        //Call static function
        if (isStatic) {
            //Get method and class names
            string methodName = this->currentNode->right->left->operation.word;
            string className = this->currentNode->right->right->operation.word;

            //Does the class exist?
            if ((*this->classes).find(className) == (*this->classes).end()) {
                throw RuntimeError("Unknown class '"+className+"'",FATAL);
            }

            newClass = (*this->classes)[className];
            method = newClass->getMethod(methodName);
            if (method == NULL) {
                throw RuntimeError("Static method '"+methodName+"' does not exist", FATAL);
            }
        } else {
            //Get the method name and variable
            string methodName = this->currentNode->right->left->operation.word;
            a = *((*this->variables)[this->currentNode->right->right->operation.word]);

            //Is the variable an object?
            if (a->getType() != 'o') {
                throw RuntimeError("Cannot call method on "+a->getTypeString(), FATAL);
            }

            //Get the method if it exists
            method = a->getMethod(methodName);
            newClass = ((Object*) a)->getClass();
            if (method == NULL) {
                throw RuntimeError("Dynamic method '"+methodName+"' does not exist", FATAL);
            }
        }

        Scope scope = Scope();
        scope.instructionPointer = this->instructionPointer - 1;
        scope.method             = this->currentMethod;
        scope.currentObject      = this->currentObject;
        scope.currentClass       = this->currentClass;
        scope.variables          = this->variables;
        scope.registerVariables  = this->registerVariables;
        scope.currentNode        = this->currentNode;
        this->scopeStack.push(scope);

        this->instructionPointer = 0;
        this->currentMethod      = method;
        this->currentObject      = a;
        this->currentClass       = newClass;
        this->registerVariables  = new stack<Variable*>();
        this->variables = new map<string, Variable**>();

        this->loadMethodParameters();

        throw FunctionCall();

        return;

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

    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::staticVar() {
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

    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::dynamicVar() {
    Variable** aP;
    Variable* a;
    Variable* result;

    //Get the class
    aP = (*this->variables)[this->currentNode->right->operation.word];
    if (aP == NULL) {
        throw RuntimeError("Cannot get property of undefined", ERROR);
    }

    a = *aP;

    if (a->getType() != 'o') {
        throw RuntimeError("Cannot get property of "+a->getTypeString(), ERROR);
    }

    //Compute result
    this->executeLeft = false;
    result = a->getProperty(this->currentNode->left->operation.word);

    if (result->getVisibility() != PUBLIC) {
        throw RuntimeError("Cannot access private variable from outside class",ERROR);
    }

    this->registerVariables->push(result);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Executor::iff() {
    Variable* a;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

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
    a = this->registerVariables->top();
    this->registerVariables->pop();

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
    a = this->registerVariables->top();
    this->registerVariables->pop();

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
    a = this->registerVariables->top();
    this->registerVariables->pop();
    b = this->registerVariables->top();
    this->registerVariables->pop();

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
    this->registerVariables->push(result);
}
