#ifndef SOURCE_OPERATIONNODE_H_
#define SOURCE_OPERATIONNODE_H_

#include "token.h"
#include <string>


class OperationNode {
    public:
        Token operation;
        OperationNode* left;
        OperationNode* right;

        OperationNode();
        ~OperationNode();

        std::string getPostfix();
};

#endif
