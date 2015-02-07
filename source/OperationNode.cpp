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
string OperationNode::getTreePlot(int level) {
    string result = "";

    int spacing = level * 4;
    for (int i = 0; i < spacing; i++) {
        result += ' ';
    }

    result += this->operation.word + '\n';

    //if (level == 3) return result;

    level += 1;

    if (this->left != NULL)
        result += this->left->getTreePlot(level);
    if (this->right != NULL)
        result += this->right->getTreePlot(level);

    return result;
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
