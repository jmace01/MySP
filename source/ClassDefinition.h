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
        std::map<std::string, Variable*> dynamicVariables;
        std::map<std::string, Variable*> staticVariables;
        std::map<std::string, Method*> methods;

    public:
        ClassDefinition();
        ~ClassDefinition();
        void addMethod(std::string &name, Method* m);
        std::map<std::string, Method*> getMethods();
        Method* getMethod(std::string &name);
};

#endif
