/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     ExpressionTreeFlattener.h
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

#ifndef SOURCE_EXPRESSIONTREEFLATTENER_H_
#define SOURCE_EXPRESSIONTREEFLATTENER_H_

#include <string>
#include <map>
#include "OperationNode.h"
#include "Instruction.h"
#include <iostream>




class ExpressionTreeFlattener {
    private:
        static std::map<std::string, InstructionCode> machineCodeMap;
        std::map<std::string, long> variableNames;
        long varCount;

    public:
        ExpressionTreeFlattener();
        virtual ~ExpressionTreeFlattener();
        void flattenTree(OperationNode* root);
        static InstructionCode getMachineCode(std::string s);

    private:
        static void initialize();
        int hashVariable(std::string s);
};

#endif
