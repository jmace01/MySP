#include "Executor.h"
#include "Method.h"
#include "Variables/Array.h"
#include "Variables/Number.h"
#include "Variables/Object.h"
#include "Variables/String.h"
#include "Variables/Variable.h"
#include <stdlib.h>
#include <iostream>

using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Method::Method(Visibility visibility, bool isStatic) {
    this->visibility = visibility;
    this->isStatic  = isStatic;
    this->parameters = std::vector< std::string >();
    this->defaultParameters = std::vector<Variable*>();
    this->instructions = vector<OperationNode*>();
}


/****************************************************************************************
 *
 ****************************************************************************************/
Method::~Method() {
    while (!this->instructions.empty()) {
        delete this->instructions.back();
        this->instructions.pop_back();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
Visibility Method::getVisibility() {
    return this->visibility;
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Method::getIsStatic() {
    return this->isStatic;
}


/****************************************************************************************
 *
 ****************************************************************************************/
unsigned long Method::getInstructionSize() {
    return this->instructions.size();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Method::addInstruction(OperationNode* op) {
    this->instructions.push_back(op);
}


/****************************************************************************************
 *
 ****************************************************************************************/
OperationNode* Method::getInstruction(unsigned long iNum) {
    return this->instructions.at(iNum);
}


/****************************************************************************************
 *
 ****************************************************************************************/
int Method::getParameterSize() {
    return this->parameters.size();
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Method::addParameter(string &name) {
    this->parameters.push_back(name);
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Method::getParameter(int i) {
    return this->parameters.at(i);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Method::addDefault(Token &t) {
    Variable* var;

    //Number
    if (t.type == 'n') {
        var = new Number(PARAMETER, strtod(t.word.c_str(), NULL));
    }
    //String
    else if (t.type == 's') {
        var = new String(PARAMETER, t.word);
    }
    //Variables or constants
    else if (t.type == 'w') {
        var = Executor::constants[t.word];
        if (var == NULL) {
            throw PostfixError("Invalid default parameter", t);
        }
    }
    //Invalid type
    else {
        throw PostfixError("Invalid default parameter", t);
    }

    this->defaultParameters.push_back(var);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Method::getDefaultParameter(int index) {
    index = index + this->defaultParameters.size() - this->parameters.size();

    if (index < this->defaultParameters.size() && index >= 0) {
        return this->defaultParameters.at(index);
    } else {
        return NULL;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
int Method::getMaxParameters() {
    return this->parameters.size();
}


/****************************************************************************************
 *
 ****************************************************************************************/
int Method::getMinParameters() {
    return (this->parameters.size() - this->defaultParameters.size());
}
