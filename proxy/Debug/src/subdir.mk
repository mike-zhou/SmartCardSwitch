################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CDeviceManager.cpp \
../src/CListener.cpp \
../src/CSocketManager.cpp \
../src/CommandTranslater.cpp \
../src/ProxyLogger.cpp \
../src/ReplyFactory.cpp \
../src/proxy.cpp 

OBJS += \
./src/CDeviceManager.o \
./src/CListener.o \
./src/CSocketManager.o \
./src/CommandTranslater.o \
./src/ProxyLogger.o \
./src/ReplyFactory.o \
./src/proxy.o 

CPP_DEPS += \
./src/CDeviceManager.d \
./src/CListener.d \
./src/CSocketManager.d \
./src/CommandTranslater.d \
./src/ProxyLogger.d \
./src/ReplyFactory.d \
./src/proxy.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/user1/Poco/poco-1.8.0.1/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


