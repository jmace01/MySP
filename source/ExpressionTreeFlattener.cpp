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

//String to Instruction
map<string, InstructionCode> ExpressionTreeFlattener::machineCodeMap = map<string, InstructionCode>();

//Instruction to operation function
map<InstructionCode, void (Executor::*)(void)> ExpressionTreeFlattener::functionMap = map<InstructionCode, void (Executor::*)(void)>();


/****************************************************************************************
 *
 ****************************************************************************************/
ExpressionTreeFlattener::~ExpressionTreeFlattener() {
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::initialize() {
    //String to code
    ExpressionTreeFlattener::machineCodeMap["print"]    = PRINT;
    ExpressionTreeFlattener::machineCodeMap["return"]   = RETURN;
    ExpressionTreeFlattener::machineCodeMap["break"]    = BREAK;
    ExpressionTreeFlattener::machineCodeMap["continue"] = CONTINUE;
    ExpressionTreeFlattener::machineCodeMap["throw"]    = THROW;
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
    ExpressionTreeFlattener::machineCodeMap["&"]        = REFERENCE;
    ExpressionTreeFlattener::machineCodeMap["~"]        = DELETION;
    ExpressionTreeFlattener::machineCodeMap["->"]       = DYNAMIC_PROPERTY;
    ExpressionTreeFlattener::machineCodeMap["::"]       = STATIC_PROPERTY;
    ExpressionTreeFlattener::machineCodeMap[":P"]       = FUNCTION_PARAMETER;
    ExpressionTreeFlattener::machineCodeMap[":C"]       = FUNCTION_CALL;
    ExpressionTreeFlattener::machineCodeMap[":jmp"]     = JUMP;
    ExpressionTreeFlattener::machineCodeMap["if"]       = JUMP_NOT_TRUE;
    ExpressionTreeFlattener::machineCodeMap["["]        = ARRAY_INDEX;
    ExpressionTreeFlattener::machineCodeMap["?"]        = JUMP_TRUE;
    ExpressionTreeFlattener::machineCodeMap["try"]      = TRY;
    ExpressionTreeFlattener::machineCodeMap["catch"]    = CATCH;

    //Code to method
    ExpressionTreeFlattener::functionMap[PRINT]                 = &Executor::print;
    ExpressionTreeFlattener::functionMap[RETURN]                = &Executor::ret;
    ExpressionTreeFlattener::functionMap[BREAK]                 = &Executor::brk;
    ExpressionTreeFlattener::functionMap[CONTINUE]              = &Executor::cont;
    ExpressionTreeFlattener::functionMap[THROW]                 = &Executor::thrw;
    ExpressionTreeFlattener::functionMap[ASSIGNMENT]            = &Executor::assignment;
    ExpressionTreeFlattener::functionMap[ADD_ASSIGN]            = &Executor::addAssign;
    ExpressionTreeFlattener::functionMap[SUBTRACT_ASSIGN]       = &Executor::subAssign;
    ExpressionTreeFlattener::functionMap[MULTIPLY_ASSIGN]       = &Executor::multAssign;
    ExpressionTreeFlattener::functionMap[DIVIDE_ASSIGN]         = &Executor::divAssign;
    ExpressionTreeFlattener::functionMap[MODULUS_ASSIGN]        = &Executor::modAssign;
    ExpressionTreeFlattener::functionMap[POWER_ASSIGN]          = &Executor::powAssign;
    ExpressionTreeFlattener::functionMap[VARIABLE_EQUALS]       = &Executor::variableEquals;
    ExpressionTreeFlattener::functionMap[TYPE_EQUALS]           = &Executor::typeEquals;
    ExpressionTreeFlattener::functionMap[EQUALS]                = &Executor::equals;
    ExpressionTreeFlattener::functionMap[NOT_VARIABLE_EQUAL]    = &Executor::notVariableEquals;
    ExpressionTreeFlattener::functionMap[NOT_TYPE_EQUAL]        = &Executor::notTypeEquals;
    ExpressionTreeFlattener::functionMap[NOT_EQUAL]             = &Executor::notEquals;
    ExpressionTreeFlattener::functionMap[LESS_THAN]             = &Executor::lessThan;
    ExpressionTreeFlattener::functionMap[LESS_THAN_OR_EQUAL]    = &Executor::lessThanEqual;
    ExpressionTreeFlattener::functionMap[GREATER_THAN]          = &Executor::greaterThan;
    ExpressionTreeFlattener::functionMap[GREATER_THAN_OR_EQUAL] = &Executor::greaterThanEqual;
    ExpressionTreeFlattener::functionMap[AND]                   = &Executor::andd;
    ExpressionTreeFlattener::functionMap[OR]                    = &Executor::orr;
    ExpressionTreeFlattener::functionMap[CONCAT]                = &Executor::cat;
    ExpressionTreeFlattener::functionMap[SUBTRACT]              = &Executor::sub;
    ExpressionTreeFlattener::functionMap[ADD]                   = &Executor::add;
    ExpressionTreeFlattener::functionMap[MULTIPLY]              = &Executor::mul;
    ExpressionTreeFlattener::functionMap[DIVIDE]                = &Executor::div;
    ExpressionTreeFlattener::functionMap[POWER]                 = &Executor::pow;
    ExpressionTreeFlattener::functionMap[MODULUS]               = &Executor::mod;
    ExpressionTreeFlattener::functionMap[INCREMENT]             = &Executor::inc;
    ExpressionTreeFlattener::functionMap[DECREMENT]             = &Executor::dec;
    ExpressionTreeFlattener::functionMap[NEGATE]                = &Executor::negate;
    ExpressionTreeFlattener::functionMap[REFERENCE]             = &Executor::reference;
    ExpressionTreeFlattener::functionMap[DELETION]              = &Executor::deletion;
    ExpressionTreeFlattener::functionMap[DYNAMIC_PROPERTY]      = &Executor::dynamicVar;
    ExpressionTreeFlattener::functionMap[STATIC_PROPERTY]       = &Executor::staticVar;
    ExpressionTreeFlattener::functionMap[FUNCTION_PARAMETER]    = &Executor::parameter;
    ExpressionTreeFlattener::functionMap[FUNCTION_CALL]         = &Executor::call;
    ExpressionTreeFlattener::functionMap[JUMP]                  = &Executor::jmp;
    ExpressionTreeFlattener::functionMap[JUMP_NOT_TRUE]         = &Executor::jmpNotTrue;
    ExpressionTreeFlattener::functionMap[ARRAY_INDEX]           = &Executor::arrayIndex;
    ExpressionTreeFlattener::functionMap[JUMP_TRUE]             = &Executor::jmpTrue;
    ExpressionTreeFlattener::functionMap[TRY]                   = &Executor::tryBlock;
    ExpressionTreeFlattener::functionMap[CATCH]                 = &Executor::catchBlock;
}


/****************************************************************************************
 *
 ****************************************************************************************/
string ExpressionTreeFlattener::lookupCode(int in) {
    map<string, InstructionCode>::iterator it;
    for (it = ExpressionTreeFlattener::machineCodeMap.begin(); it != ExpressionTreeFlattener::machineCodeMap.end(); it++) {
        if (it->second == in) {
            return it->first;
        }
    }
    return "<ERROR>";
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
void ExpressionTreeFlattener::setOperationFunction(InstructionCode i, void (Executor::*&ptr)(void)) {
    ptr = ExpressionTreeFlattener::functionMap[i];
}


/****************************************************************************************
 * Changes variable name into a long
 ****************************************************************************************/
long ExpressionTreeFlattener::hashVariable(string s) {
    if (variableNames.find(s) == variableNames.end()) {
        variableNames[s] = varCount;
        variableLookup[varCount] = s;
        return varCount++;
    } else {
        return variableNames[s];
    }
}


/****************************************************************************************
 * Changes variable name into a long
 ****************************************************************************************/
map<long, string> ExpressionTreeFlattener::getVariableMap() {
    return this->variableLookup;
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::addOperand(OperationNode* node, Instruction &inst, bool sideA, bool hash) {
    //Can set
    //hash = false;
    //to stop hashing

    if (node->operation.type == 'n') {
        double dval = stod(node->operation.word);
        if (sideA) {
            inst.aType = 'n';
            inst.operandAd = dval;
        } else {
            inst.bType = 'n';
            inst.operandBd = dval;
        }
    } else if (node->operation.type == 'w' && hash && node->operation.word != "true" && node->operation.word != "false") {
        double dval = this->hashVariable(node->operation.word);
        if (sideA) {
            inst.aType = 'h'; //hash
            inst.operandAd = dval;
        } else {
            inst.bType = 'h'; //hash
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
 * ExpressionTreeFlattener::flattenTree
 *
 * Description:
 *     This method takes a binary expression tree and changes it to pseudo-assembly code
 *     that, in this program, is simply called an "Instruction" (see the Instruction.h
 *     file).
 *
 * Inputs:
 *     OperationNode* root : The root of the binary expression tree to flatten
 *     vector<Instruction> &instructionVector : The vector to put instructions into
 *     unsigned long lastCount : Used for recursive purposes. Set to 0 initially.
 *             This value shows the placement of the last node in the instructions
 *             and is needed for some jumps to map properly.
 *
 * Outputs:
 *     None (see second parameter in inputs)
 *
 * Examples:
 *
 *     Non-Short Circuiting (jumps not needed)
 *     a || b                a && b
 *
 *       ||                    &&
 *      /  \                  /  \
 *     1    1                1    1
 *
 *     a = #0, b = #1
 *     OR #1, #0               AND #1, #0
 *
 * --------------------------------------------------------------------------------------
 *
 *     Short circuiting
 *     a() || b()            a() && b()
 *
 *         ||                   &&
 *        /  \                 /  \
 *      C      C             C      C
 *       \      \             \      \
 *        a      b             a      b
 *
 *     a = #0, b = #1
 *     CALL       #0   , -   CALL           #0    , -
 *     JUMP_TRUE  <REG>, 3   JUMP_NOT_TRUE  <REG> , 3
 *     CALL       #1   , -   CALL           #1    , -
 *
 * --------------------------------------------------------------------------------------
 *
 *     Ternary Statements
 *     a = b ? 1 : 2;
 *
 *        =
 *      /   \
 *     a     ?
 *         /   \
 *       b      :
 *            /   \
 *           1     2
 *
 *     b = #0, a = #1
 *     0 | JUMP_TRUE    #0   , 3
 *     1 | ASSIGMENT    <REG>, 2
 *     2 | JUMP         4    , -
 *     3 | ASSIGNMENT   <REG>, 1
 *     4 | ASSIGNMENT   #1   , <REG>
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenTree(OperationNode* root, vector<Instruction> &instructionVector, unsigned long lastCount) {
    // Get instruction data
    Instruction inst;
    inst.instruction = ExpressionTreeFlattener::getMachineCode(root->operation.word);
    ExpressionTreeFlattener::setOperationFunction(inst.instruction, inst.opFunction); //Get function to execute
    inst.line = root->operation.line;

    //Save current position of instruction going into vector
    unsigned long currentInst = instructionVector.size();

    //What side of the tree do we go down first?
    OperationNode* firstSide = (inst.isTerminating() || root->operation.word == "?") ? root->right : root->left;
    //What side of the tree do we go down second?
    OperationNode* secondSide = (inst.isTerminating() || root->operation.word == "?") ? root->left : root->right;

    //Deal with ternary branch
    if (root->operation.word == ":") {
        //Is the first side a leaf node? If so, assign the value to a register
        if (firstSide->isLeafNode()) {
            inst.instruction = ASSIGNMENT;
            this->addOperand(firstSide, inst, false, true);
            instructionVector.push_back(inst);
        } else {
            //Not a leaf node, traverse it
            flattenTree(firstSide, instructionVector, currentInst);
        }

        //Add jump statement to the end of the statement to go over alternate branch
        inst = Instruction();
        inst.instruction = JUMP;
        inst.aType    = 'n'; //location of jump added later
        currentInst = instructionVector.size();
        instructionVector.push_back(inst);
        inst = Instruction();

        //Fix the conditional jump from parent node (ie, the "?" node)
        instructionVector[lastCount].operandBd = instructionVector.size();
        instructionVector[lastCount].bType = 'n';

        //Is the second side a leaf node? If so, assign the value to a register
        if (secondSide->isLeafNode()) {
            inst.instruction = ASSIGNMENT;
            this->addOperand(secondSide, inst, false, true);
            instructionVector.push_back(inst);
        } else {
            //Not a leaf node, traverse it
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
            bool canHash = (root->operation.word != "->" && root->operation.word != "::" && root->operation.word != ":C");
            bool sideA   = (root->operation.isUnary || root->operation.word == "?");
            this->addOperand(firstSide, inst, sideA, canHash);
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
            bool canHash = (root->operation.word != ":C");
            this->addOperand(secondSide, inst, true, canHash);
        } else {
            //Not a leaf node below, recurse down it
            flattenTree(secondSide, instructionVector, currentInst);
        }
    }

    if (inst.instruction == UNKNOWN && root->isLeafNode()) {
        inst.instruction = ASSIGNMENT;
        this->addOperand(root, inst, true, true);
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


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenMethod(Method &method) {
    //Get instruction vector
    vector<Instruction> &instructionVector = method.getInstructionCodeVector();

    //Map tree root nodes to vector indexes for jump conversions
    map<unsigned long, unsigned long> treeMap = map<unsigned long, unsigned long>();
    //Pointer to root node of tree
    OperationNode* op;
    //Insturction for creating jumps
    Instruction inst;
    //For holding positions
    unsigned long x;

    //Keep track of jumps that need to be corrected
    stack<unsigned long> jumps = stack<unsigned long>();

    //Begin flattening trees
    for (unsigned long i = 0; i < method.getInstructionTreeSize(); i++) {
        //Save root node position in new vector
        treeMap[i] = instructionVector.size() - 1;
        //Get root node
        op = method.getInstruction(i);

        //Is this a jump? If so, add a new jump instruction
        if (op->operation.word == ":jmp" || op->operation.word == "if") {
            x = stol(op->right->operation.word);
            inst.instruction = (op->operation.word == ":jmp") ? JUMP : JUMP_NOT_TRUE;
            inst.aType       = 'n';
            inst.operandAd   = x;
            instructionVector.push_back(inst);
            jumps.push(instructionVector.size() - 1);
        } else {
            //Not a jump, send to flatten tree method
            this->flattenTree(op, instructionVector, 0);
        }

        //Remove the tree
        delete op;
    }

    //Correct jumps now that we have all the instructions
    unsigned long val;
    while (!jumps.empty()) {
        //Get the original value of the jump location
        x = instructionVector[jumps.top()].operandAd;
        //Translate the old location to the new location
        if (treeMap.find(x) != treeMap.end()) {
            //The old location is in the vector
            val = treeMap[x] + (instructionVector[jumps.top()].instruction != JUMP);
        } else {
            //The old location is at the end of the vector
            val = instructionVector.size();
        }
        //Update the instruction
        instructionVector[jumps.top()].operandAd = val;
        jumps.pop();
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenClass(ClassDefinition &cls) {
    map<string, Method*>::iterator it;
    map<string, Method*> methods = cls.getMethods();

    for (it = methods.begin(); it != methods.end(); it++) {
        this->flattenMethod(*(it->second));
    }
}


/****************************************************************************************
 *
 ****************************************************************************************/
void ExpressionTreeFlattener::flattenClassMap(map<string, ClassDefinition* >* &classMap) {
    map<string, ClassDefinition* >::iterator it;

    for (it = classMap->begin(); it != classMap->end(); it++) {
        this->flattenClass(*(it->second));
    }
}
