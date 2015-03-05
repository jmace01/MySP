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
        std::map<std::string, Method*> methods;

    public:
        ClassDefinition();
        ~ClassDefinition();
        void addProperty(std::string &name, Variable &v);
        void addMethod(std::string &name, Method* m);
        std::map<std::string, Method*> getMethods();
        Method* getMethod(std::string &name);
        void setInheritance(ClassDefinition* inherClass);
};

#endif