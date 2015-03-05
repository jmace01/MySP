#include <string>
#include "Variable.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Variable::Variable(Visibility visibility, bool isStatic) {
    this->visibility = visibility;
    this->isStatic   = isStatic;
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
char Variable::getType() {
    return 'u'; //NULL
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Variable::getTypeString() {
    return "null"; //Undefined
}


/****************************************************************************************
 *
 ****************************************************************************************/
float Variable::getNumberValue() {
    return 0;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Variable::getStringValue() {
    return "NULL";
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::getArrayValue() {
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator= (Variable &rhs) {
    throw RuntimeError("Cannot use = operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator+ (Variable &rhs) {
    throw RuntimeError("Cannot use + operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator- (Variable &rhs) {
    throw RuntimeError("Cannot use - operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator++ (int) {
    throw RuntimeError("Cannot use ++ operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator-- (int) {
    throw RuntimeError("Cannot use -- operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator* (Variable &rhs) {
    throw RuntimeError("Cannot use * operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator/ (Variable &rhs) {
    throw RuntimeError("Cannot use / operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator% (Variable &rhs) {
    throw RuntimeError("Cannot use % operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::operator[] (int i) {
    throw RuntimeError("Cannot use index operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::operator== (Variable &rhs) {
    return (rhs.getType() == 'u');
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Variable::operator< (Variable &rhs) {
    return (rhs.getType() != 'u');
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::power(Variable &rhs) {
    throw RuntimeError("Cannot use ^ operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Method* Variable::getMethod(std::string, bool isStatic) {
    string op = (isStatic) ? "::" : "->";
    throw RuntimeError("Cannot use "+op+" operator on type '"+this->getTypeString()+"'");
    return NULL;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Variable::getPropery(std::string, bool isStatic) {
    string op = (isStatic) ? "::" : "->";
    throw RuntimeError("Cannot use "+op+" operator on type '"+this->getTypeString()+"'");
    return new Variable(PUBLIC, false);
}