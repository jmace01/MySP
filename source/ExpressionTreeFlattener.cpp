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
    ExpressionTreeFlattener::machineCodeMap["if"]       = JUMP_NOT_TRUE;
    ExpressionTreeFlattener::machineCodeMap["["]        = ARRAY_INDEX;
    ExpressionTreeFlattener::machineCodeMap["?"]        = JUMP_TRUE;
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
void ExpressionTreeFlattener::addOperand(OperationNode* node, Instruction &inst, bool sideA) {
    if (node->operation.type == 'n') {
        double dval = stod(node->operation.word);
        if (sideA) {
            inst.aType = 'n';
            inst.operandAd = dval;
        } else {
            inst.bType = 'n';
            inst.operandBd = dval;
        }
    } else {
        if (sideA){
            inst.aType = node->operation.type;
            inst.operandAs = node->operation.word;
        } else {
            inst.bType = node->operation.type;
            inst.operandBs = node->operation.word;
        }
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenTree(OperationNode* root, vector<Instruction> &instructionVector, unsigned long lastCount) {
    // Get instruction data
    Instruction inst;
    inst.instruction = ExpressionTreeFlattener::getMachineCode(root->operation.word);
    inst.opFunction  = NULL; //Get function to execute
    inst.line = root->operation.line;

    //Save current position of instruction going into vector
    unsigned long currentInst = instructionVector.size();

    //What side of the tree do we go down first?
    OperationNode* firstSide = (inst.isTerminating() || root->operation.word == "?") ? root->right : root->left;
    //What side of the tree do we go down second?
    OperationNode* secondSide = (inst.isTerminating() || root->operation.word == "?") ? root->left : root->right;;

    //Deal with ternary branch
    if (root->operation.word == ":") {
        //Is the first side a leaf node? If so, create a LOAD instruction for it
        if (firstSide->isLeafNode()) {
            inst.instruction = LOAD;
            this->addOperand(firstSide, inst, true);
            instructionVector.push_back(inst);
        }
        //Not a leaf node, traverse it
        else {
            flattenTree(firstSide, instructionVector, currentInst);
        }
        //Add jump statement to the end of the statement to go over alternate branch
        inst.instruction = JUMP;
        inst.aType    = 'n'; //location of jump added later
        currentInst = instructionVector.size();
        instructionVector.push_back(inst);
        //Fix the conditional jump from parent node (ie, the "?" node)
        instructionVector[lastCount].operandBd = instructionVector.size();
        instructionVector[lastCount].bType = 'n';
        //Is the second side a leaf node? If so, create a LOAD instruction for it
        if (secondSide->isLeafNode()) {
            inst.instruction = LOAD;
            this->addOperand(secondSide, inst, true);
            instructionVector.push_back(inst);
        }
        //Not a leaf node, traverse it
        else {
            flattenTree(secondSide, instructionVector, currentInst);
        }
        //Fix jump that was previously added
        instructionVector[currentInst].operandAd = instructionVector.size();
        return;
    }

    //Check the first side to see if a node is below it
    if (firstSide != NULL) {
        //If the side is a leaf node, put its value as an operand on the current
        //operation instead of going down it
        if (firstSide->isLeafNode()) {
            //Assign the operand
            this->addOperand(firstSide, inst, (root->operation.isUnary || root->operation.word == "?"));
        } else {
            //Not a leaf node below, recurse down it
            flattenTree(firstSide, instructionVector, currentInst);
        }
    }

    //If this is a short circuiting operation, put the instruction before
    //second side to allow the instructions to jump past the second side
    if ((inst.isTerminating() || root->operation.word == "?") && secondSide != NULL && !secondSide->isLeafNode()) {
        instructionVector.push_back(inst);
    }

    //Check the second side to see if there is a node below it
    if (secondSide != NULL) {
        //If the side is a leaf node, put its value as an operand on the current
        //operation instead of going down it
        if (secondSide->isLeafNode()) {
            this->addOperand(secondSide, inst, true);
        } else {
            //Not a leaf node below, recurse down it
            flattenTree(secondSide, instructionVector, currentInst);
        }
    }

    //If the instruction is not terminating, put it here
    if ((!inst.isTerminating() && root->operation.word != "?") || secondSide == NULL || secondSide->isLeafNode()) {
        instructionVector.push_back(inst);
    }
    //If the instruction was terminating, update it so that is knows where to jump to
    else if (root->operation.word != "?") {
        instructionVector[currentInst + 1].operandBd = instructionVector.size();
        instructionVector[currentInst + 1].bType = 'n';
        instructionVector[currentInst + 1].convertToJump(); //change instruction to jump
    }
}

// Examples given as follows:
//
// Title
// Code
// Binary Expression Tree
// MySP Instruction Code
//
// ---------------------------------------------
//
// Non-Short Circuiting (s.c. not needed)
//
// 1 || 1              1 && 1
//
//   ||                  &&
//  /  \                /  \
// 1    1              1    1
//
// OR 1, 1             AND 1, 1
//
// ---------------------------------------------
//
// Short circuiting
//
// a() || b()          a() && b()
//
//     ||                 &&
//    /  \               /  \
//  C      C           C      C
//   \      \           \      \
//    a      b           a      b
//
// CALL       a, -     CALL           a     , -
// JUMP_TRUE  @, 3     JUMP_NOT_TRUE  <REG> , 3
// CALL       b, -     CALL           b     , -
//
// ---------------------------------------------
//
// Ternary Statements
//
// a = b ? 1 : 2;
//
//    =
//  /   \
// a     ?
//     /   \
//   b      :
//        /   \
//       1     2
//
// 0 | JUMP_TRUE    b, 3
// 1 | LOAD         2, -
// 2 | JUMP         4, -
// 3 | LOAD         1, -
// 4 | ASSIGNMENT   a, <REG>
