################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/MySP.cpp \
../source/OperationNode.cpp \
../source/postfix.cpp 

OBJS += \
./source/MySP.o \
./source/OperationNode.o \
./source/postfix.o 

CPP_DEPS += \
./source/MySP.d \
./source/OperationNode.d \
./source/postfix.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


