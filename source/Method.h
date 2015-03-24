#ifndef SOURCE_METHOD_H_
#define SOURCE_METHOD_H_

#include <map>
#include <string>
#include <vector>
#include "OperationNode.h"
#include "Variables/Variable.h"


class Method {
    private:
        Visibility visibility;
        bool isStatic;
        std::vector<std::string>    parameters;
        std::vector<Variable*>      defaultParameters;
        std::vector<OperationNode*> instructions;

    public:
        Method(Visibility visibility, bool isStatic);
        ~Method();
        Visibility getVisibility();
        bool getIsStatic();
        unsigned long getInstructionSize();
        void addInstruction(OperationNode* op);
        OperationNode* getInstruction(unsigned long iNum);
        void addParameter(std::string &name);
        int getParameterSize();
        std::string getParameter(int i);
        void addDefault(Token &t);
        Variable* getDefaultParameter(int index);
        int getMaxParameters();
        int getMinParameters();
};

#endif
