/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     ExpressionTreeFlattener.cpp
 *
 * DESCRIPTION:
 *
 *
 * AUTHOR:
 *     Jason Mace
 *
 *
 * Copyright 2015 by Jason Mace
 *
 */////////////////////////////////////// !! ////////////////////////////////////////////

#include <functional>
#include "ExpressionTreeFlattener.h"

using namespace std;


/****************************************************************************************
 *
 ****************************************************************************************/
ExpressionTreeFlattener::ExpressionTreeFlattener() {
    varCount = 0;

    if (machineCodeMap.empty()) {
        ExpressionTreeFlattener::initialize();
    }
}

map<string, InstructionCode> ExpressionTreeFlattener::machineCodeMap = map<string, InstructionCode>();


/****************************************************************************************
 *
 ****************************************************************************************/
ExpressionTreeFlattener::~ExpressionTreeFlattener() {
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::initialize() {
    ExpressionTreeFlattener::machineCodeMap["print"]    = PRINT;
    ExpressionTreeFlattener::machineCodeMap["return"]   = RETURN;
    ExpressionTreeFlattener::machineCodeMap["break"]    = BREAK;
    ExpressionTreeFlattener::machineCodeMap["continue"] = CONTINUE;
    ExpressionTreeFlattener::machineCodeMap["="]        = ASSIGNMENT;
    ExpressionTreeFlattener::machineCodeMap["+="]       = ADD_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["-="]       = SUBTRACT_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["*="]       = MULTIPLY_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["/="]       = DIVIDE_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["%="]       = MODULUS_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["^="]       = POWER_ASSIGN;
    ExpressionTreeFlattener::machineCodeMap["===="]     = VARIABLE_EQUALS;
    ExpressionTreeFlattener::machineCodeMap["==="]      = TYPE_EQUALS;
    ExpressionTreeFlattener::machineCodeMap["=="]       = EQUALS;
    ExpressionTreeFlattener::machineCodeMap["!==="]     = NOT_VARIABLE_EQUAL;
    ExpressionTreeFlattener::machineCodeMap["!=="]      = NOT_TYPE_EQUAL;
    ExpressionTreeFlattener::machineCodeMap["!="]       = NOT_EQUAL;
    ExpressionTreeFlattener::machineCodeMap["<"]        = LESS_THAN;
    ExpressionTreeFlattener::machineCodeMap["<="]       = LESS_THAN_OR_EQUAL;
    ExpressionTreeFlattener::machineCodeMap[">"]        = GREATER_THAN;
    ExpressionTreeFlattener::machineCodeMap[">="]       = GREATER_THAN_OR_EQUAL;
    ExpressionTreeFlattener::machineCodeMap["&&"]       = AND;
    ExpressionTreeFlattener::machineCodeMap["||"]       = OR;
    ExpressionTreeFlattener::machineCodeMap["."]        = CONCAT;
    ExpressionTreeFlattener::machineCodeMap["-"]        = SUBTRACT;
    ExpressionTreeFlattener::machineCodeMap["+"]        = ADD;
    ExpressionTreeFlattener::machineCodeMap["*"]        = MULTIPLY;
    ExpressionTreeFlattener::machineCodeMap["/"]        = DIVIDE;
    ExpressionTreeFlattener::machineCodeMap["%"]        = MODULUS;
    ExpressionTreeFlattener::machineCodeMap["^"]        = POWER;
    ExpressionTreeFlattener::machineCodeMap["++"]       = INCREMENT;
    ExpressionTreeFlattener::machineCodeMap["--"]       = DECREMENT;
    ExpressionTreeFlattener::machineCodeMap["!"]        = NEGATE;
    ExpressionTreeFlattener::machineCodeMap["->"]       = DYNAMIC_PROPERTY;
    ExpressionTreeFlattener::machineCodeMap["::"]       = STATIC_PROPERTY;
    ExpressionTreeFlattener::machineCodeMap["P"]        = FUNCTION_PARAMETER;
    ExpressionTreeFlattener::machineCodeMap["C"]        = FUNCTION_CALL;
    ExpressionTreeFlattener::machineCodeMap["jmp"]      = JUMP;
    ExpressionTreeFlattener::machineCodeMap["if"]       = IFF;
    ExpressionTreeFlattener::machineCodeMap["["]        = ARRAY_INDEX;
    ExpressionTreeFlattener::machineCodeMap["?"]        = TERNARY;
}


/****************************************************************************************
 *
 ****************************************************************************************/
InstructionCode ExpressionTreeFlattener::getMachineCode(string s) {
    return ExpressionTreeFlattener::machineCodeMap[s];
}


/****************************************************************************************
 *
 ****************************************************************************************/
int ExpressionTreeFlattener::hashVariable(string s) {
    if (variableNames.find(s) == variableNames.end()) {
        variableNames[s] = varCount;
        return varCount++;
    } else {
        return variableNames[s];
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenTree(OperationNode* root) {
    Instruction inst;
    inst.operandA = "@"; //register
    inst.operandB = "@"; //register

    if (root->left != NULL) {
        if (root->left->isLeafNode()) {
            inst.operandB = root->left->operation.word;
        } else {
            flattenTree(root->left);
        }
    }

    if (root->right != NULL) {
        if (root->right->isLeafNode()) {
            inst.operandA = root->right->operation.word;
        } else {
            flattenTree(root->right);
        }
    }

    inst.instruction = ExpressionTreeFlattener::getMachineCode(root->operation.word);
    cout << root->operation.word << " " << inst.instruction << " " << inst.operandA << ", " << inst.operandB << endl;
}
