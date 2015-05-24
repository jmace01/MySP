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
    , IFF                   // iff
    , ARRAY_INDEX           // [
    , TERNARY               // ?
};




struct Instruction {
    InstructionCode instruction;
    std::string operandA;
    std::string operandB;
};

#endif
