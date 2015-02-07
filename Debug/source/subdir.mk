################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/ExpressionTreeBuilder.cpp \
../source/MySP.cpp \
../source/OperationNode.cpp 

OBJS += \
./source/ExpressionTreeBuilder.o \
./source/MySP.o \
./source/OperationNode.o 

CPP_DEPS += \
./source/ExpressionTreeBuilder.d \
./source/MySP.d \
./source/OperationNode.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


