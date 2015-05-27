/*/////////////////////////////////////// !! ////////////////////////////////////////////
 *
 * FILE:
 *     Instruction.h
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

#ifndef SOURCE_INSTRUCTION_H_
#define SOURCE_INSTRUCTION_H_

#include <string>
#include "Executor.h"

enum InstructionCode {
      UNKNOWN               // << For unknown operators >>
    , CLASS_LABEL           // << For marking classes in saved file >>
    , METHOD_LABEL          // << For marking methods in saves file >>
    , PRINT                 // print
    , RETURN                // return
    , BREAK                 // break
    , CONTINUE              // continue
    , ASSIGNMENT            // =
    , ADD_ASSIGN            // +=
    , SUBTRACT_ASSIGN       // -=
    , MULTIPLY_ASSIGN       // *=
    , DIVIDE_ASSIGN         // /=
    , MODULUS_ASSIGN        // %=
    , POWER_ASSIGN          // ^=
    , VARIABLE_EQUALS       // ====
    , TYPE_EQUALS           // ===
    , EQUALS                // =
    , NOT_VARIABLE_EQUAL    // !===
    , NOT_TYPE_EQUAL        // !==
    , NOT_EQUAL             // !=
    , LESS_THAN             // <
    , LESS_THAN_OR_EQUAL    // <=
    , GREATER_THAN          // >
    , GREATER_THAN_OR_EQUAL // >=
    , AND                   // &&
    , OR                    // ||
    , CONCAT                // .
    , SUBTRACT              // -
    , ADD                   // +
    , MULTIPLY              // *
    , DIVIDE                // /
    , MODULUS               // %
    , POWER                 // ^
    , INCREMENT             // ++
    , DECREMENT             // --
    , NEGATE                // !
    , DYNAMIC_PROPERTY      // ->
    , STATIC_PROPERTY       // ::
    , FUNCTION_PARAMETER    // P
    , FUNCTION_CALL         // C
    , JUMP                  // jmp
    , JUMP_TRUE             //
    , JUMP_NOT_TRUE         //
    , ARRAY_INDEX           // [
    , LOAD                  // << Used in ternary statements for end values that have no operations >>
};




struct Instruction {
    Instruction() :
        line(0),
        opFunction(NULL),
        instruction(UNKNOWN),
        operandAs("@"),
        operandBs("@"),
        operandAd(0),
        operandBd(0),
        aType('w'),
        bType('w') {}

    int line;
    void (Executor::*opFunction)(void);

    InstructionCode instruction;

    //Strings
    std::string operandAs;
    std::string operandBs;
    //Numbers
    double operandAd;
    double operandBd;
    //Types
    char aType;
    char bType;

    /**
     * Functions
     */

    bool isTerminating() {
        return (
                instruction == AND ||
                instruction == OR
        );
    }

    void convertToJump() {
        switch (instruction) {
            case AND:
                instruction = JUMP_NOT_TRUE;
            break;
            case OR:
                instruction = JUMP_TRUE;
            break;
            default:
                return;
        }
    }
};

#endif
