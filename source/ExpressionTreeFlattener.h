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
#include <vector>
#include "ClassDefinition.h"
#include "method.h"
#include "OperationNode.h"
#include "Instruction.h"
#include <iostream>
#include "Executor.h"




class ExpressionTreeFlattener {
    private:
        static std::map<std::string, InstructionCode> machineCodeMap;
        static std::map<InstructionCode, void (Executor::*)(void)> functionMap;
        std::map<std::string, long> variableNames;
        std::map<long, std::string> variableLookup;
        long varCount;

    public:
        ExpressionTreeFlattener();
        virtual ~ExpressionTreeFlattener();
        void flattenTree(OperationNode* root, std::vector<Instruction> &instructionVector, unsigned long lastCount);
        static InstructionCode getMachineCode(std::string s);
        void setOperationFunction(InstructionCode i, void (Executor::*&ptr)(void));
        void addOperand(OperationNode* node, Instruction &inst, bool sideA, bool hash);
        void flattenMethod(Method &method);
        void flattenClass(ClassDefinition &cls);
        void flattenClassMap(std::map<std::string, ClassDefinition* >* &classMap);
        static std::string lookupCode(int in);
        std::map<long, std::string> getVariableMap();

    private:
        static void initialize();
        long hashVariable(std::string s);
};

#endif
