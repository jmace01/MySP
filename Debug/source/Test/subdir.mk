################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/Test/ExecutorTest.cpp \
../source/Test/ExpressionTreeBuilderTest.cpp \
../source/Test/ParserTest.cpp 

OBJS += \
./source/Test/ExecutorTest.o \
./source/Test/ExpressionTreeBuilderTest.o \
./source/Test/ParserTest.o 

CPP_DEPS += \
./source/Test/ExecutorTest.d \
./source/Test/ExpressionTreeBuilderTest.d \
./source/Test/ParserTest.d 


# Each subdirectory must supply rules for building sources it contributes
source/Test/%.o: ../source/Test/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


