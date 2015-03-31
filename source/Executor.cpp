/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     Executor.cpp
 *
 * DESCRIPTION:
 *
 *     This file contains all of the methods needed to execute the binary expression
 *     tress created by the Parser and BinaryExpressionTreeBuilder classes.
 *
 *     This file starts execution (after being constructed) with the Executor::run
 *     method.
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////

#include "Executor.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 * Executor::Executor()
 *
 * Description:
 *     Executor Constructor.
 *     Initializes the member variables for the Executor class.
 *     Most member variables are initialized with each call to Executor::run.
 *     See that method for more information.
 *
 * Notes:
 *     Many member variables are only initialized here to prevent warning messages
 *     when the code is compiled.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
Executor::Executor() {
    //Execution and clean up
    this->instructionPointer = 0;   //What instruction are we executing?
    this->deleteClasses = true;     //Should classes be deleted when destructor is called?
    this->currentMethod = NULL;     //The current method being executed
    this->currentObject = NULL;     //Saves object for "this" variable
    this->currentClass  = NULL;     //Class currently executing method
    this->classes = NULL;           //The class definitions from the Parser

    //Scope variables
    this->scopeStack = stack<Scope>();                //The scope for function calls
    this->registerVariables = new stack<Variable*>(); //The temporary values for expressions
    this->parameterStack = stack<Variable*>();        //Parameters for method calls
    this->variables = NULL;                           //The variables to be stored

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
 * Executor::~Executor
 *
 * Description:
 *     Executor Destructor.
 *     1. Removes classes if specified.
 *     2. Frees any memory left in the remaining variable map (the rest should have popped
 *        off with Scope, see Executor::run.
 *     3. Frees constant variables.
 *     4. Frees any values that are still in the register stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
Executor::~Executor() {
    //Remove classes if set
    if (this->deleteClasses && this->classes != NULL) {
        map<string, ClassDefinition* >::iterator it;
        for (it = this->classes->begin(); it != this->classes->end(); it++) {
            delete it->second;
        }
    }

    //Remove constants
    map<string, Variable*>::iterator cit;
    for (cit = this->constants.begin(); cit != this->constants.end(); cit++) {
        delete cit->second;
    }
    Executor::constants = map<string, Variable*>();

    //Remove register data
    this->clearRegisters();
}


/****************************************************************************************
 * Executor::initializeOperationMap
 *
 * Description:
 *     Creates a map that will translate string containing operations into a pointer to
 *     the method that will execute that operation.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::initializeConstants
 *
 * Description:
 *     Creates a map that takes a string containing a constant keyword and gives a
 *     variable back with the value of that constant value.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::initializeConstants() {
    this->constants["false"]          = new Number(CONST, 0);
    this->constants["true"]           = new Number(CONST, 1);
    this->constants["null"]           = new Nil(CONST);
    this->constants["pi"]             = new Number(CONST, 3.14159);
}


/****************************************************************************************
 * Executor::run
 *
 * Description:
 *     The driver for the executor Executor class. This method:
 *     1. Initialized the running environment for execution.
 *     2. Begins executing at the main method of the ~ class.
 *         This is a special class and method created in the Parser class.
 *     3. While there are still instructions to be executed, the method will execute
 *         instructions one at a time until:
 *         I.  There is an error
 *         II. A function is called
 *     The Scope and instruction pointer can be set in other methods.
 *
 * Inputs:
 *     map<string, ClassDefinition* >* classes : The classes that will be executed by the
 *         Executor class.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::run(map<string, ClassDefinition* >* classes) {
    //If constants do not exist, initialize them
    if (this->constants.empty()) {
        this->initializeConstants();
    }

    //Set up the instruction pointer to the first instruction
    this->instructionPointer = 0;
    //Save classes to member variable
    this->classes = classes;
    //Set up initial variable map
    this->variables = new map<string, Variable**>();

    //Set up initial method call to entry point (main)
    string startMethod = "main";
    this->currentMethod = (*this->classes)["~"]->getMethod(startMethod);
    this->scopeStack.push(Scope());

    //Used for recovering recursion after method call
    string recover = "";

    //Begin executing instructions
    while (!scopeStack.empty()) {
        //If the method end has been reached, pop off the method and get the previous one
        if (instructionPointer >= this->currentMethod->getInstructionSize()) {
            //There are methods to return to
            if (this->scopeStack.size() > 1) {
                //Remove old variables
                this->clearVariables();
                //If this was a dynamic method call, reset "this" as a normal variable
                if (this->currentObject != NULL) {
                    this->currentObject->makeNonConstant();
                }
                //Get previous scope
                Scope scope = this->scopeStack.top();
                this->instructionPointer = scope.instructionPointer;
                this->variables = scope.variables;
                this->registerVariables = scope.registerVariables;
                this->currentMethod = scope.method;
                this->currentObject = scope.currentObject;
                this->currentClass  = scope.currentClass;
                this->currentNode = scope.currentNode;
                //Get previous position in recursion
                recover = this->recoverPosition(this->currentMethod->getInstruction(this->instructionPointer), '0');
                //Does the line execution need to be finished?
                if (recover != "" && recover != "0") {
                    //Put the return value in the register
                    if (!scope.isConstructor) {
                        if (this->returnVariable != NULL) {
                            this->registerVariables->push(this->returnVariable);
                        }
                        //Default return value
                        else {
                            this->registerVariables->push(new Nil(TEMP));
                        }
                    } else {
                        //Remove temporary pointer
                        delete (*this->variables)["this"];
                    }
                }
                //The last line was finished, move on
                else {
                    if (this->returnVariable != NULL && !scope.isConstructor) {
                        delete this->returnVariable;
                    }
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
        }
        //There was a runtime error
        catch (RuntimeError &e) {
            this->displayError(e);
            //Avoid memory leaks in registers
            this->clearRegisters();
            if (e.level == FATAL) {
                //Avoid memory leaks in variable memory
                do {
                    this->variables = this->scopeStack.top().variables;
                    this->clearVariables();
                    scopeStack.pop();
                } while (!this->scopeStack.empty());
                return;
            }
        }
        //Function calls are thrown to break out of recursion
        catch (FunctionCall &e) {}

        recover = "";
    }
}


/****************************************************************************************
 * Executor::clearVariables
 *
 * Description:
 *     Frees the memory used by variables in a map. This method is called when returning
 *     scope after a user method call has ended.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::clearVariables() {
    //Nothing to do -- this case should never happen but is here as a precaution
    if (this->variables == NULL) {
        return;
    }

    //Delete map contents
    map<string, Variable**>::iterator it;
    for (it = this->variables->begin(); it != this->variables->end(); it++) {
        if ((*it->second)->getVisibility() != CONST) {
            delete (*it->second);
            delete it->second;
        }
    }

    //Delete map
    delete this->variables;
}


/****************************************************************************************
 * Executor::loadValue
 *
 * Description:
 *     Takes an operator node and puts the associated value onto the registerVariables
 *     stack. The value is based on the operation.type attribute.
 *
 * Inputs:
 *     OperationNode * op : The operation node containing a constant value, variable
 *         name, string, or number.
 *
 * Outputs:
 *     None
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
        if (Executor::constants.find(op->operation.word) != Executor::constants.end()) {
            var = Executor::constants[op->operation.word];
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
 * Executor::executeInstruction
 *
 * Description:
 *     Recursively executes the instruction in an expression tree by performing a postfix
 *     depth-first traversal. Special cases are needed when the operation is terminating,
 *     meaning that one or more branches should only be executed if another branch met a
 *     specified condition (for example, &&, || and ? operations).
 *
 *     The recover string is used for getting back to end continuing execution after a
 *     method call forces the recursion to abort before completing.
 *
 *     The recovery makes the code considerably more obfuscated but was needed because
 *     the trees are not flattened before execution.
 *
 * Inputs:
 *     OperationNode* op : The operation node traverse from
 *     string recover : The directions to the starting point if recovering from a method
 *         call.
 *
 * Outputs:
 *     None
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
            this->executeInstruction(op->left, "");
        }

        //Get right node
        if (op->right != NULL && !recoverRight) {
            this->executeInstruction(op->right, "");
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
 * Executor::executeOperator
 *
 * Description:
 *     Uses the member OperationMap to translate the input operation into a method
 *     pointer that is then called to execute the operation.
 *
 *     Operation that are not found in the map are skipped.
 *
 * Inputs:
 *     OperationNode* op : The operation to execute
 *
 * Outputs:
 *     None
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
 * Executor::recoverPosition
 *
 * Description:
 *     Searches the expression tree to find the node that was being executed when a
 *     method was called.
 *
 *     The method being searched for is in the currentNode member variable.
 *
 *     The directions are in this form:
 *         0 = Root node
 *         L = Left child
 *         R = Right child
 *
 *     Examples "0LLR" : Go to left child, left child, right child and begin execution
 *              "0" : Begin at the root
 *
 * Inputs:
 *     OperationNode* op : The node to traverse down from to search for the currentNode.
 *     char direction : Begin with "0" for the root node and append letters for the
 *         direction the node was found at.
 *
 * Outputs:
 *     string : The directions to the node being operated on.
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
 * Executor::clearRegisters
 *
 * Description:
 *     Frees the memory being used by variables in the registerVariables map.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::displayError
 *
 * Description:
 *     Displays an error message.
 *
 * Inputs:
 *     RuntimeError &e : The error message to display
 *
 * Outputs:
 *     None
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
 * Executor::preserveClasses
 *
 * Description:
 *     Sets whether or not the destructor should delete the class definitions used in
 *     the execution of the script.
 *
 * Inputs:
 *     bool preserve : If the classes should kept (in other words, should NOT be deleted).
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::preserveClasses(bool preserve) {
    this->deleteClasses = !preserve;
}


/****************************************************************************************
 * Executor::initMethodCall
 *
 * Description:
 *     Prepares a method call for execution by:
 *     1. Checking that the method can legally be called.
 *     2. Saving the current scope.
 *     3. Creating a new scope.
 *     4. Adding a "this" member variable to the new scope, if needed.
 *     5. Adding parameters to the new scope.
 *     6. Throwing a FunctionCall struct to break recursion.
 *
 * Inputs:
 *     Method* method : The method to call.
 *     Variable* object : The object to call the method from (if dynamic). Also used to
 *         moderate scope.
 *     ClassDefinition* classDef : The class definition to call the method from (if
 *         static). Also used to moderate scope.
 *     bool isStatic : Is the method call static or dynamic? If true, static.
 *     bool isConstructor : Is the method a constructor? If true, yes.
 *     string &methodName : The name of the method in case of an error.
 *
 * Outputs:
 *     None
 ****************************************************************************************/
inline void Executor::initMethodCall(
        Method* method, Variable* object, ClassDefinition* classDef,
        bool isStatic, bool isConstructor, string &methodName )
{
    //Can the method be called legally?
    string constructorError = isConstructor ? "constructor " : "";
    if (method->getVisibility() != PUBLIC &&
            (
                (!isStatic && object != this->currentObject)
                ||
                (isStatic && this->currentClass != classDef)
            )
        )
    {
        throw RuntimeError("Permission denied to call "+constructorError+"'"+methodName+"'", FATAL);
    } else if (!isStatic && method->getIsStatic()) {
        throw RuntimeError("Cannot call static method "+constructorError+"'"+methodName+"' in dynamic way", FATAL);
    } else if (isStatic && !method->getIsStatic()) {
        throw RuntimeError("Cannot call dynamic method "+constructorError+"'"+methodName+"' in static way", FATAL);
    }

    //Save the current scope
    Scope scope = Scope();
    scope.instructionPointer = this->instructionPointer - 1;
    scope.method             = this->currentMethod;
    scope.currentObject      = this->currentObject;
    scope.currentClass       = this->currentClass;
    scope.variables          = this->variables;
    scope.registerVariables  = this->registerVariables;
    scope.currentNode        = this->currentNode;
    scope.isConstructor      = isConstructor;
    this->scopeStack.push(scope);

    //Set up new scope
    this->instructionPointer = 0;
    this->currentMethod      = method;
    this->currentObject      = object;
    this->currentClass       = classDef;
    this->registerVariables  = new stack<Variable*>();
    this->variables = new map<string, Variable**>();

    //Create "this" object
    if (this->currentObject != NULL) {
        if (!isConstructor) {
            (*this->variables)["this"] = this->currentObject->getPointer();
        } else {
            Variable** vPointer = new Variable*;
            *vPointer = this->currentObject;
            this->currentObject->setPointer(vPointer);
            (*this->variables)["this"] = vPointer;
        }
        this->currentObject->makeConstant();
    }

    //Get parameters for method
    this->loadMethodParameters();

    //Start the new method by breaking out of recursion with a thrown object
    throw FunctionCall();
}


/****************************************************************************************
 * Executor::print()
 *
 * Description:
 *     Prints out the top value of the registerVarialbes stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::ret()
 *
 * Description:
 *     Ends a method call by:
 *     1. If there is a return value, set the returnVariable pointer to point to it.
 *     2. Set the instruction pointer to one past the last instruction to force the scope
 *         to change in the Executor::run method.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::makeVariableCopy
 *
 * Description:
 *     Creates a copy of an input variable with the input visibility.
 *
 * Inputs:
 *     Variable* v : The variable to copy.
 *     Visibility visibility : The visibility of the new variable copy.
 *
 * Outputs:
 *     Variable* : The copy of the variable that was created.
 ****************************************************************************************/
Variable* Executor::makeVariableCopy(Variable* v, Visibility visibility) {
    Variable* result;

    if (v->getType() == 'n') {
        result = new Number(visibility, v->getNumberValue());
    } else if (v->getType() == 's') {
        string s = v->getStringValue();
        result = new String(visibility, s);
    } else if (v->getType() == 'a') {
        result = new Array(visibility, (Array*) v);
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
 * Executor::assignment
 *
 * Description:
 *     Takes the first two variables ("a" and "b") from the registerVariables stack and
 *     creates a new variables with the value of b and the visibility of a and saves it
 *     in place of a in the member map of variables.
 *
 *     So for the following code:
 *         var = 1;
 *     The value of var (a) is replaced with a new variable created with the value 1 (b)
 *     with the visibility of the old var (a) to ensure scope remains the same.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::variableEquals
 *
 * Description:
 *     Checks that the top two values on the registerVariables stack are the exact same
 *     variable.
 *
 *     Example: a ==== a //true
 *              a ==== b //false
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::typeEquals
 *
 * Description:
 *     Checks that the top two variables of the registerVariables stack have the same
 *     value and the same type.
 *
 *     Example: 1 === 1   //true
 *              1 === '1' //false, types are different
 *              1 === 2   //false, values are different
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::equals
 *
 * Description:
 *     Checks that the top two variables of the registerVariables stack have the same
 *     value.
 *
 *     Example: 1 == 1   //true
 *              1 == '1' //true
 *              1 == 2   //false, values are different
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::notVariableEquals
 *
 * Description:
 *     Checks that the top two values on the registerVariables stack are not the exact
 *     same variable.
 *
 *     Example: a !=== a //false
 *              a !=== b //true
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::notTypeEquals
 *
 * Description:
 *     Checks that the top two variables of the registerVariables stack do not have the
 *     same value or the same type.
 *
 *     Example: 1 !== 1   //false
 *              1 !== '1' //true, types are different
 *              1 !== 2   //true, values are different
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::notEquals
 *
 * Description:
 *     Checks that the top two variables of the registerVariables stack do not have the
 *     same value.
 *
 *     Example: 1 != 1   //false
 *              1 != '1' //false
 *              1 != 2   //true, values are different
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::lessThan
 *
 * Description:
 *     Checks that the top variable (a) of the registerVariables stack has a lower value
 *     than the second variable (b) of the registerVariables stack.
 *
 *     Example: 1 < 1    //false
 *              1 < 2    //true
 *              NULL < 1 //true
 *              1 < NULL //false
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::lessThanEqual
 *
 * Description:
 *      Checks that the top variable (a) of the registerVariables stack has a lower or
 *      equal value to the second variable (b) of the registerVariables stack.
 *
 *     Example: 1 <= 1    //true
 *              1 <= 2    //true
 *              NULL <= 1 //true
 *              1 <= NULL //false
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::greaterThan
 *
 * Description:
 *      Checks that the top variable (a) of the registerVariables stack has a higher
 *      value than the second variable (b) of the registerVariables stack.
 *
 *     Example: 2 > 1    //true
 *              2 > 2    //false
 *              NULL > 1 //false
 *              1 > NULL //true
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::greaterThanEqual
 *
 * Description:
 *     Checks that the top variable (a) of the registerVariables stack has a higher
 *     equal value to the second variable (b) of the registerVariables stack.
 *
 *     Example: 2 >= 1    //true
 *              2 >= 2    //true
 *              NULL >= 1 //false
 *              1 >= NULL //true
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::andd
 *
 * Description:
 *     Checks that the top variable (a) of the registerVariables stack is true. If it is
 *     true, the other side of the branch is permitted to be executed. If not, the branch
 *     short circuits.
 *
 *     Example: 0 && 0    //false, short circuit
 *              0 && 1    //false, short circuit
 *              1 && 0    //true, execute other side
 *              1 && 1    //true, execute other side
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::orr
 *
 * Description:
 *     Checks that the top variable (a) of the registerVariables stack is true. If it is
 *     false, the other side of the branch is permitted to be executed. If not, the branch
 *     short circuits.
 *
 *     Example: 0 || 0    //true, execute other side
 *              0 || 1    //true, execute other side
 *              1 || 0    //false, short circuit (but return true)
 *              1 || 1    //false, short circuit (but return true)
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::add
 *
 * Description:
 *     Adds the second variable (b) on the registerVariables stack to the first variable
 *     (a) on the registerVaraibles stack and puts the result on the registerVaraibles
 *     stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::sub
 *
 * Description:
 *     Subtracts the second variable (b) on the registerVariables stack from the first
 *     variable (a) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::mul
 *
 * Description:
 *     Multiplies the second variable (b) on the registerVariables stack with the first
 *     variable (a) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::div
 *
 * Description:
 *     Divides the first variable (a) on the registerVariables stack by the second
 *     variable (b) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::mod
 *
 * Description:
 *     Modulo's the first variable (a) on the registerVariables stack by the second
 *     variable (b) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::pow
 *
 * Description:
 *     Raises the first variable (a) on the registerVariables stack to the power of the
 *     second variable (b) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::cat
 *
 * Description:
 *     Concatenates the second variable (b) on the registerVariables stack to the end of
 *     the first variable (a) on the registerVaraibles stack and puts the result on the
 *     registerVaraibles stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::inc
 *
 * Description:
 *     Increments the first variable (a) on the registerVariables stack and puts the old
 *     value onto the registerVariables stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::dec
 *
 * Description:
 *     Decrements the first variable (a) on the registerVariables stack and puts the old
 *     value onto the registerVariables stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::negate
 *
 * Description:
 *     Performs a boolean not on the first variable (a) of the registerVariables stack
 *     and puts the result on the registerVariables stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::parameter
 *
 * Description:
 *     Takes the top variable (a) of the registerVariables stack and puts in on the
 *     parameter stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::parameter() {
    Variable* a;

    //Get operand values
    a = this->registerVariables->top();
    this->registerVariables->pop();

    this->parameterStack.push(a);
}


/****************************************************************************************
 * Executor::loadMethodParameters
 *
 * Description:
 *     Loads the parameters and default parameter values into the current scope by taking
 *     and provided values on the parameterStack and merging them with default values
 *     provided with the currentMethod.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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

    for (int j = this->currentMethod->getParameterSize() - 1; j >= 0; j--) {
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
 * Executor::call
 *
 * Description:
 *     Handles method calls and constructors. Cases are:
 *     1. A static method call (foo::bar())
 *     2. A dynamic method call (foo->bar())
 *     3. An array constructor (array())
 *     4. A object constructor (foo())
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::call() {
    OperationNode* nextOp;
    Variable* a = NULL;
    Variable* result;

    //Compute result
    //this->executeLeft = false;
    nextOp = this->currentNode->right;

    bool isStatic = nextOp->operation.word == "::";
    bool isDynamic = nextOp->operation.word == "->";

    //Is this a method call?
    if (isStatic || isDynamic) {
        Method* method;
        ClassDefinition* newClass;
        string methodName;

        //Get the method information
        if (isStatic) {
            //Get method and class names
            methodName = this->currentNode->right->left->operation.word;
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
            methodName = this->currentNode->right->left->operation.word;

            //Get variable pointer
            Variable** vPointer;

            if (this->currentNode->right->right->operation.word == "->") {
                OperationNode* oldCurrent = this->currentNode;
                this->currentNode = this->currentNode->right->right;
                dynamicVar();
                a = this->registerVariables->top();
                this->registerVariables->pop();
                this->currentNode = oldCurrent;
            }
            else {
                //Get the variable pointer
                vPointer = (*this->variables)[this->currentNode->right->right->operation.word];

                //Does the variable exist?
                if (vPointer == NULL) {
                    throw RuntimeError("Cannot call method on undefined", FATAL);
                }

                //Get the variable
                a = *((*this->variables)[this->currentNode->right->right->operation.word]);
            }

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

        //Change scope and properties for new method call
        this->initMethodCall(method, a, newClass, isStatic, false, methodName);

        //Leave function
        return;

    } else {

        //Create an array
        if (this->currentNode->right->operation.word == "array") {
            result = new Array(TEMP);
        }

        //Create a new class instance
        else {
            string methodName = this->currentNode->right->operation.word;
            if ((*this->classes).find(methodName) == this->classes->end()) {
                throw RuntimeError("No class found", FATAL);
            }
            result = new Object(TEMP, (*this->classes)[methodName]);
            ClassDefinition* classDef = ((Object*) result)->getClass();

            Method* constructor = classDef->getMethod(methodName);
            if (constructor != NULL) {
                this->registerVariables->push(result);
                this->initMethodCall(constructor, result, classDef, false, true, methodName);
                return;
            }
        }

    }

    this->registerVariables->push(result);
}


/****************************************************************************************
 * Executor::staticVar
 *
 * Description:
 *     Gets a static variable by getting the class from the right child node and the
 *     property from the left child node. The result is put on the registerVariables
 *     stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::staticVar() {
    Variable* result;

    //Get the class name
    string className = this->currentNode->right->operation.word;

    //Compute result
    //this->executeLeft = false;
    if ((*this->classes).find(className) == (*this->classes).end()) {
        throw RuntimeError("Unknown class '"+className+"'",ERROR);
    }
    ClassDefinition* cls = (*this->classes)[className];
    result = cls->getStaticProperty(this->currentNode->left->operation.word);

    if (result->getVisibility() != PUBLIC && cls != this->currentClass) {
        throw RuntimeError("Cannot access private variable from outside class",ERROR);
    }

    this->registerVariables->push(result);
}


/****************************************************************************************
 * Executor::dynamicVar
 *
 * Description:
 *     Gets a dynamic variable by getting the object from the right child node and the
 *     property from the left child node. The result is put on the registerVariables
 *     stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
 ****************************************************************************************/
void Executor::dynamicVar() {
    Variable** aP;
    Variable* a;
    Variable* result;

    //Get the object
    //Chained -> with -> or ::
    if (this->currentNode->right->operation.word == "->" || this->currentNode->right->operation.word == "::") {
        bool isDynamic = (this->currentNode->right->operation.word == "->");
        OperationNode* oldCurrent = this->currentNode;
        this->currentNode = this->currentNode->right;
        if (isDynamic) {
            dynamicVar();
        }
        else {
            staticVar();
        }
        a = this->registerVariables->top();
        this->registerVariables->pop();
        this->currentNode = oldCurrent;
    }
    //Variable
    else {
        aP = (*this->variables)[this->currentNode->right->operation.word];
        if (aP == NULL) {
            throw RuntimeError("Cannot get property of undefined", ERROR);
        }

        a = *aP;
    }

    if (a->getType() != 'o') {
        throw RuntimeError("Cannot get property of "+a->getTypeString(), ERROR);
    }

    //Compute result
    //this->executeLeft = false;
    result = a->getProperty(this->currentNode->left->operation.word);

    if (result->getVisibility() != PUBLIC && this->currentObject != a) {
        throw RuntimeError("Cannot access private variable from outside class",ERROR);
    }

    this->registerVariables->push(result);
}


/****************************************************************************************
 * Executor::iff
 *
 * Description:
 *     Conditionally jumps to a line. If the value of lastValue (which was the final
 *     value of the previous expression tree that was executed) is false, jump to the
 *     top value of the registerVariables stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::jmp
 *
 * Description:
 *     An unconditional jump. Jumps to the top value of the registerVariables stack.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::ternary
 *
 * Description:
 *     Decided which part of a ternary expression to execute.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
 * Executor::arrayIndex
 *
 * Description:
 *     Gets the value of an array index and puts it unto the registerVariables stack.
 *     The top value (a) is the array and the second value (b) is the index.
 *
 * Inputs:
 *     None
 *
 * Outputs:
 *     None
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
