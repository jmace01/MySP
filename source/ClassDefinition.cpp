#include "ClassDefinition.h"

using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition::ClassDefinition() {
    this->inheritedClass   = NULL;
    this->dynamicVariables = map<string, Variable*>();
    this->staticVariables  = map<string, Variable*>();
    this->methods          = map<string, Method*>();
}


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition::~ClassDefinition() {
    map<string, Variable*>::iterator it;
    map<string, Method*>::iterator it2;
    //Remove any dynamic variable
    for (it = dynamicVariables.begin(); it != dynamicVariables.end(); it++) {
        delete it->second;
    }
    //Remove any static variables
    for (it = staticVariables.begin(); it != staticVariables.end(); it++) {
        delete it->second;
    }
    //Remove methods
    for (it2 = methods.begin(); it2 != methods.end(); it2++) {
        delete it2->second;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::addMethod(string &name, Method* m) {
    //Does the method already exist?
    if (this->methods.find(name) != this->methods.end()) {
        throw PostfixError("Redefinition of method '"+name+"'");
    }

    //Create the method
    else {
        this->methods[name] = m;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
map<string, Method*> ClassDefinition::getMethods() {
    return this->methods;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Method* ClassDefinition::getMethod(string &name) {
    //If the method is not in this class, look at inherited classes
    if (this->methods.find(name) == this->methods.end()) {
        //There is an inherited class, check in it
        if (this->inheritedClass != NULL) {
            return this->inheritedClass->getMethod(name);
        }
        //There is no inherited class, return NULL
        else {
            return NULL;
        }
    }
    //If the method is in this class, return it
    else {
        return this->methods[name];
    }
}
