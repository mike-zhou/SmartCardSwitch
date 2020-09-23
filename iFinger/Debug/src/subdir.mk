################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CommandRunner.cpp \
../src/UserListener.cpp \
../src/WebServer.cpp \
../src/iFinger.cpp 

OBJS += \
./src/CommandRunner.o \
./src/UserListener.o \
./src/WebServer.o \
./src/iFinger.o 

CPP_DEPS += \
./src/CommandRunner.d \
./src/UserListener.d \
./src/WebServer.d \
./src/iFinger.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/mikez/Poco/poco-1.9.0/include -I"/home/user1/Developments/invenco/Logger/include" -I"/home/user1/Developments/invenco/MsgPackager/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


