################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Command.cpp \
../src/CommandFactory.cpp \
../src/CommandRunner.cpp \
../src/ConsoleCommandFactory.cpp \
../src/ConsoleOperator.cpp \
../src/CoordinateStorage.cpp \
../src/DeviceAccessor.cpp \
../src/MovementConfiguration.cpp \
../src/ReplyTranslator.cpp \
../src/SmartCardSwitch.cpp \
../src/UserCommandRunner.cpp \
../src/UserListener.cpp \
../src/UserProxy.cpp \
../src/WebServer.cpp 

OBJS += \
./src/Command.o \
./src/CommandFactory.o \
./src/CommandRunner.o \
./src/ConsoleCommandFactory.o \
./src/ConsoleOperator.o \
./src/CoordinateStorage.o \
./src/DeviceAccessor.o \
./src/MovementConfiguration.o \
./src/ReplyTranslator.o \
./src/SmartCardSwitch.o \
./src/UserCommandRunner.o \
./src/UserListener.o \
./src/UserProxy.o \
./src/WebServer.o 

CPP_DEPS += \
./src/Command.d \
./src/CommandFactory.d \
./src/CommandRunner.d \
./src/ConsoleCommandFactory.d \
./src/ConsoleOperator.d \
./src/CoordinateStorage.d \
./src/DeviceAccessor.d \
./src/MovementConfiguration.d \
./src/ReplyTranslator.d \
./src/SmartCardSwitch.d \
./src/UserCommandRunner.d \
./src/UserListener.d \
./src/UserProxy.d \
./src/WebServer.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/mikez/Poco/poco-1.9.0/include -I/home/mikez/Developments/invenco/MsgPackager/include -I/home/mikez/Developments/invenco/Logger/include -O0 -g3 -Wall -c -fmessage-length=0  -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


