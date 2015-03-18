#include "Array.h"
#include "Number.h"
#include "Object.h"
#include "String.h"


using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
Object::Object(Visibility visibility, ClassDefinition* cd)
    : Variable(visibility)
{
    //Save the class definition
    this->classDef = cd;

    //Create Object Properties
    this->properties = map<string, Variable**>();
    map<string, Variable> vars = cd->getProperties();
    map<string, Variable>::iterator it;
    Variable** vPointer = NULL;
    Variable* var;
    for (it = vars.begin(); it != vars.end(); it++) {
        var = new Variable(it->second.getVisibility());
        vPointer = new Variable*;
        *vPointer = var;
        this->properties[it->first] = vPointer;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
Object::Object(Visibility visibility, Object* obj)
    : Variable(visibility)
{
    //Save the class definition
    this->classDef = obj->classDef;

    //Create Object Properties
    this->properties = map<string, Variable**>();
    map<string, Variable**> vars = obj->properties;
    map<string, Variable**>::iterator it;
    Variable** vPointer = NULL;
    Variable* var;
    for (it = vars.begin(); it != vars.end(); it++) {
        vPointer = new Variable*;
        if ((*it->second)->getType() == 'n') {
            var = new Number((*it->second)->getVisibility(), (*it->second)->getNumberValue());
        } else if ((*it->second)->getType() == 's') {
            string s = (*it->second)->getStringValue();
            var = new String((*it->second)->getVisibility(), s);
        } else if ((*it->second)->getType() == 'a') {
            var = new Array((*it->second)->getVisibility());
        } else if ((*it->second)->getType() == 'o') {
            var = new Object((*it->second)->getVisibility(), (Object*) (*it->second));
        } else {
            var = new Variable((*it->second)->getVisibility());
        }
        *vPointer = var;
        var->setPointer(vPointer);
        this->properties[it->first] = vPointer;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
Object::~Object() {
    map<string, Variable**>::iterator it;
    for (it = this->properties.begin(); it != this->properties.end(); it++) {
        delete (*it->second);
        delete it->second;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
char Object::getType() {
    return 'o';
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Object::getTypeString() {
    return "object";
}


/****************************************************************************************
 *
 ****************************************************************************************/
double Object::getNumberValue() {
    return 0;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string Object::getStringValue() {
    return "OBJECT";
}


/****************************************************************************************
 *
 ****************************************************************************************/
bool Object::getBooleanValue() {
    return true;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* Object::getProperty(string index) {
    if (this->properties.find(index) == this->properties.end()) {
        throw RuntimeError("Dynamic property '"+index+"' of class does not exist", ERROR);
    }

    return *(this->properties[index]);
}


/****************************************************************************************
 *
 ****************************************************************************************/
Method* Object::getMethod(string index) {
    Method* m = this->classDef->getMethod(index);

    if (m == NULL) {
        throw RuntimeError("Method '"+index+"' of class does not exist", FATAL);
    }

    return m;
}
