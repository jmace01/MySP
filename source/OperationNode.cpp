/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     OperationNode.cpp
 *
 * DESCRIPTION:
 *     The fundamental unit of an Binary Expression Tree.
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////


#include "OperationNode.h"
#include <iostream>
#include <string>


using namespace std;


/****************************************************************************************
 * Constructor
 ****************************************************************************************/
OperationNode::OperationNode() {
    this->left = NULL;
    this->right = NULL;
}


/****************************************************************************************
 * Destructor -- Deletes children
 ****************************************************************************************/
OperationNode::~OperationNode() {
    if (left != NULL) {
        delete this->left;
    }
    if (right != NULL) {
        delete this->right;
    }
}


/****************************************************************************************
 * Is this node a leaf node?
 ****************************************************************************************/
bool OperationNode::isLeafNode() {
    return (this->left == NULL && this->right == NULL);
}


/****************************************************************************************
 * Used for debugging purposes
 ****************************************************************************************/
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


/****************************************************************************************
 * Used for unit testing
 ****************************************************************************************/
string OperationNode::getPostfix() {
    string result;
    if (this->left != NULL)
        result = this->left->getPostfix() + ' ';

    if (this->right != NULL)
        result += this->right->getPostfix() + ' ';

    return result + this->operation.word;
}
