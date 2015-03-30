/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     Array.cpp
 *
 * DESCRIPTION:
 *     Array Variable Type
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////


#include "Array.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Array::Array(Visibility visibility)
    : Variable(visibility)
{
    this->values = map<string, Variable**>();
}


/****************************************************************************************
 *
 ****************************************************************************************/
Array::Array(Visibility visibility, Array* array)
    : Variable(visibility)
{
    Variable** vPointer;
    Variable* var;
    this->values = map<string, Variable**>();
    map<string, Variable**> aMap = array->getArrayValueMap();
    map<string, Variable**>::iterator it;
    for (it = aMap.begin(); it != aMap.end(); it++) {
        vPointer = new Variable*;
        var = Executor::makeVariableCopy((*it->second), (*it->second)->getVisibility());
        *vPointer = var;
        var->setPointer(vPointer);
        this->values[it->first] = vPointer;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
Array::~Array() {
    map<string, Variable**>::iterator it;
    for (it = this->values.begin(); it != this->values.end(); it++) {
        delete (*it->second);
        delete it->second;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Array::getType() {
    return 'a';
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Array::getTypeString() {
    return "string";
}


/****************************************************************************************
 *
 ****************************************************************************************/
double Array::getNumberValue() {
    return 0;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Array::getStringValue() {
    return "ARRAY";
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Array::getBooleanValue() {
    return (this->values.size() > 0);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Array::getArrayValue(string index) {
    Variable** vPointer = NULL;
    Variable* var;

    if (this->values.find(index) == this->values.end()) {
        var = new Variable(PUBLIC);
        vPointer = new Variable*;
        *vPointer = var;
        var->setPointer(vPointer);
        this->values[index] = vPointer;
    }

    return *(this->values[index]);
}


/****************************************************************************************
 *
 ****************************************************************************************/
std::map<std::string, Variable**> Array::getArrayValueMap() {
    return this->values;
}
