#ifndef SOURCE_CLASSDEFINITION_H_
#define SOURCE_CLASSDEFINITION_H_

#include <map>
#include <string>
#include <vector>
#include "Method.h"
#include "OperationNode.h"
#include "ExpressionTreeBuilder.h"
#include "Variables/Variable.h"

class ClassDefinition {
    private:
        ClassDefinition* inheritedClass;
        std::map<std::string, Variable> properties;
        std::map<std::string, Variable**> staticProperties;
        std::map<std::string, Method*> methods;

    public:
        ClassDefinition();
        ~ClassDefinition();
        void addProperty(std::string &name, Variable &v, Token &t);
        void addStaticProperty(std::string &name, Variable &v, Token &t);
        void addMethod(std::string &name, Method* m, Token &t);
        std::map<std::string, Method*> getMethods();
        Method* getMethod(std::string &name);
        std::map<std::string, Variable> getProperties();
        void setInheritance(ClassDefinition* inherClass);
        Variable* getStaticProperty(std::string index);
        ClassDefinition* getInheritedClass();
};

#endif
