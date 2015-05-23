################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/ClassDefinition.cpp \
../source/Executor.cpp \
../source/ExpressionTreeBuilder.cpp \
../source/ExpressionTreeFlattener.cpp \
../source/Method.cpp \
../source/MySP.cpp \
../source/OperationNode.cpp \
../source/Parser.cpp \
../source/Tokenizer.cpp 

OBJS += \
./source/ClassDefinition.o \
./source/Executor.o \
./source/ExpressionTreeBuilder.o \
./source/ExpressionTreeFlattener.o \
./source/Method.o \
./source/MySP.o \
./source/OperationNode.o \
./source/Parser.o \
./source/Tokenizer.o 

CPP_DEPS += \
./source/ClassDefinition.d \
./source/Executor.d \
./source/ExpressionTreeBuilder.d \
./source/ExpressionTreeFlattener.d \
./source/Method.d \
./source/MySP.d \
./source/OperationNode.d \
./source/Parser.d \
./source/Tokenizer.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


