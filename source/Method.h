#ifndef SOURCE_METHOD_H_
#define SOURCE_METHOD_H_

#include <vector>
#include "OperationNode.h"

class Method {
    private:
        bool isPrivate;
        bool isStatic;
        std::vector<OperationNode*> instructions;

    public:
        Method(bool isPrivate, bool isStatic);
        ~Method();
        bool getIsPivate();
        bool getIsStatic();
        unsigned long getInstructionSize();
        void addInstruction(OperationNode* op);
        OperationNode* getInstruction(unsigned long iNum);
};

#endif
