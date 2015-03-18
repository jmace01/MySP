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
void Method::addParameter(string &name) {
    this->parameters.push_back(name);
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Method::addDefault(Token &t) {
    Variable* v;
    if (t.type == 's') {
        v = new String(PUBLIC, t.word);
    } else if (t.type == 'n') {
        v = new Number(PUBLIC, strtof(t.word.c_str(), NULL));
    }
}
