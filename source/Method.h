#ifndef SOURCE_METHOD_H_
#define SOURCE_METHOD_H_

#include <vector>
#include "OperationNode.h"


enum Visibility {PRIVATE, PUBLIC, PROTECTED};


class Method {
    private:
        Visibility visibility;
        bool isStatic;
        std::vector<OperationNode*> instructions;

    public:
        Method(Visibility visibility, bool isStatic);
        ~Method();
        Visibility getVisibility();
        bool getIsStatic();
        unsigned long getInstructionSize();
        void addInstruction(OperationNode* op);
        OperationNode* getInstruction(unsigned long iNum);
};

#endif
