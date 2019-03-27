################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CommandFactory.cpp \
../src/scsClient.cpp 

OBJS += \
./src/CommandFactory.o \
./src/scsClient.o 

CPP_DEPS += \
./src/CommandFactory.d \
./src/scsClient.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/mikez/Poco/poco-1.9.0/include -I"/home/mikez/Developments/invenco/scsClient/include" -I"/home/mikez/Developments/invenco/Logger/include" -I"/home/mikez/Developments/invenco/MsgPackager/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


