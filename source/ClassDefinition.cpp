/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     ClassDefinition.cpp
 *
 * DESCRIPTION:
 *     Holds the class methods and properties as defined by the user.
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////


#include "ClassDefinition.h"
#include <iostream>

using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition::ClassDefinition() {
    this->inheritedClass   = NULL;
    this->properties = map<string, Variable>();
    this->staticProperties = map<string, Variable**>();
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
    //Remove static variables
    map<string, Variable**>::iterator sit;
    for (sit = staticProperties.begin(); sit != staticProperties.end(); sit++) {
        delete (*sit->second);
        delete sit->second;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::addProperty(string &name, Variable &v, Token &t) {
    //Does the property already exist?
    if (this->properties.find(name) != this->properties.end()) {
        throw PostfixError("Redefinition of property '"+name+"'", t);
    } else if (this->staticProperties.find(name) != this->staticProperties.end()) {
        throw PostfixError("Redefinition of property '"+name+"'", t);
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
void ClassDefinition::addStaticProperty(string &name, Variable &v, Token &t) {
    //Does the property already exist?
    if (this->properties.find(name) != this->properties.end()) {
            throw PostfixError("Redefinition of property '"+name+"'", t);
    } else if (this->staticProperties.find(name) != this->staticProperties.end()) {
            throw PostfixError("Redefinition of property '"+name+"'", t);
    }

    //Create the method
    else {
        //Don't use default constructor
        Variable** vPointer = new Variable*;
        Variable* var = new Variable(v.getVisibility());
        *vPointer = var;
        var->setPointer(vPointer);
        this->staticProperties[name] = vPointer;
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::addMethod(string &name, Method* m, Token &t) {
    //Does the method already exist?
    if (this->methods.find(name) != this->methods.end()) {
        throw PostfixError("Redefinition of method '"+name+"'", t);
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
            Method* method = this->inheritedClass->getMethod(name);
            //Only return public and protected methods
            if (method != NULL && method->getVisibility() == PRIVATE) {
                return NULL;
            }
            return method;
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
map<string, Variable> ClassDefinition::getProperties() {
    return this->properties;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ClassDefinition::setInheritance(ClassDefinition* inherClass) {
    this->inheritedClass = inherClass;
}


/****************************************************************************************
 *
 ****************************************************************************************/
Variable* ClassDefinition::getStaticProperty(string index) {
    if (this->staticProperties.find(index) == this->staticProperties.end()) {
        throw RuntimeError("Static property '"+index+"' of class does not exist", ERROR);
    }

    return *(this->staticProperties[index]);
}


/****************************************************************************************
 *
 ****************************************************************************************/
ClassDefinition* ClassDefinition::getInheritedClass() {
    return this->inheritedClass;
}
