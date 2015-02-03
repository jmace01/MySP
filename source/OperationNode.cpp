#include "OperationNode.h"
#include <iostream>
#include <string>


using namespace std;


/******************************************************************
 *
 ******************************************************************/
OperationNode::OperationNode() {
    this->left = NULL;
    this->right = NULL;
}


/******************************************************************
 *
 ******************************************************************/
OperationNode::~OperationNode() {
    if (left != NULL) {
        delete this->left;
    }
    if (right != NULL) {
        delete this->right;
    }
}


/******************************************************************
 *
 ******************************************************************/
string OperationNode::getPostfix() {
    string result;
    if (this->left != NULL)
        result = this->left->getPostfix() + ' ';

    if (this->right != NULL)
        result += this->right->getPostfix() + ' ';

    return result + this->operation.word;
}
