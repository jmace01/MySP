#include "ClassDefinition.h"

using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition::ClassDefinition() {
    this->inheritedClass   = NULL;
    this->properties = map<string, Variable>();
    this->methods    = map<string, Method*>();
}


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition::~ClassDefinition() {
    map<string, Method*>::iterator it;
    //Remove methods
    for (it = methods.begin(); it != methods.end(); it++) {
        delete it->second;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::addProperty(string &name, Variable &v) {
    //Does the property already exist?
    if (this->properties.find(name) != this->properties.end()) {
        throw PostfixError("Redefinition of property '"+name+"'");
    }

    //Create the method
    else {
        //Don't use default constructor (properties[name] = v)
        this->properties.insert(std::pair<string,Variable>(name,v));
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


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::setInheritance(ClassDefinition* inherClass) {
    this->inheritedClass = inherClass;
}
