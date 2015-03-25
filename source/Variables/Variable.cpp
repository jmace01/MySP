#include <string>
#include "Variable.h"
#include <iostream>


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Variable::Variable(Visibility visibility) {
    this->preConstVisibility = visibility;
    this->visibility         = visibility;
    this->pointer            = NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable::~Variable() {
    //
}


/****************************************************************************************
 *
 ****************************************************************************************/
Visibility Variable::getVisibility() {
    return this->visibility;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Variable::setPointer(Variable** pointer) {
    this->pointer = pointer;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable** Variable::getPointer() {
    return this->pointer;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Variable::makeConstant() {
    if (this->preConstVisibility != CONST) {
        this->preConstVisibility = this->visibility;
    }
    this->visibility = CONST;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void Variable::makeNonConstant() {
    this->visibility = this->preConstVisibility;
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Variable::getType() {
    return 'u'; //Undefined
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Variable::getTypeString() {
    return "undefined"; //Undefined
}


/****************************************************************************************
 *
 ****************************************************************************************/
double Variable::getNumberValue() {
    return 0;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Variable::getStringValue() {
    return "UNDEFINED";
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::getBooleanValue() {
    return false;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::getArrayValue(string index) {
    throw RuntimeError("Cannot index '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::getProperty(string index) {
    throw RuntimeError("Cannot get property of '"+this->getTypeString()+"'", FATAL);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Method* Variable::getMethod(string index) {
    throw RuntimeError("Cannot get method of '"+this->getTypeString()+"'", FATAL);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator= (Variable &rhs) {
    //If possible, copy over the value
    //

    //Create new variable
    Variable* var;

    var = new Variable(TEMP);

    delete this;
    return var;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator+ (Variable &rhs) {
    throw RuntimeError("Cannot use + operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator- (Variable &rhs) {
    throw RuntimeError("Cannot use - operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator++ (int) {
    throw RuntimeError("Cannot use ++ operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator-- (int) {
    throw RuntimeError("Cannot use -- operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator* (Variable &rhs) {
    throw RuntimeError("Cannot use * operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator/ (Variable &rhs) {
    throw RuntimeError("Cannot use / operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator% (Variable &rhs) {
    throw RuntimeError("Cannot use % operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator[] (int i) {
    throw RuntimeError("Cannot use index operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::power(Variable &rhs) {
    throw RuntimeError("Cannot use ^ operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::concat(Variable &rhs) {
    throw RuntimeError("Cannot use '.' operator on type '"+this->getTypeString()+"'", WARNING);
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::operator== (Variable &rhs) {
    return !((getType() == 'u' && rhs.getType() != 'u') ||
            (getType() != 'u' && rhs.getType() == 'u') ||
            getStringValue() != rhs.getStringValue());
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::operator< (Variable &rhs) {
    if (getType() == 'n' && rhs.getType() == 'n') {
        return getNumberValue() < rhs.getNumberValue();
    }
    return !((this->getType() == 'u' && rhs.getType() == 'u') ||
            (this->getType() != 'u' && rhs.getType() == 'u') ||
            this->getStringValue() >= rhs.getStringValue());
}

/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::operator> (Variable &rhs) {
    if (getType() == 'n' && rhs.getType() == 'n') {
        return getNumberValue() > rhs.getNumberValue();
    }
    return !((this->getType() == 'u' && rhs.getType() == 'u') ||
            (this->getType() == 'u' && rhs.getType() != 'u') ||
            this->getStringValue() <= rhs.getStringValue());
}
