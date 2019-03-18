################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CDeviceManager.cpp \
../src/CDeviceMonitor.cpp \
../src/CListener.cpp \
../src/CSocketManager.cpp \
../src/CommandTranslater.cpp \
../src/ProxyLogger.cpp \
../src/ReplyFactory.cpp \
../src/ReplyTranslater.cpp \
../src/proxy.cpp 

OBJS += \
./src/CDeviceManager.o \
./src/CDeviceMonitor.o \
./src/CListener.o \
./src/CSocketManager.o \
./src/CommandTranslater.o \
./src/ProxyLogger.o \
./src/ReplyFactory.o \
./src/ReplyTranslater.o \
./src/proxy.o 

CPP_DEPS += \
./src/CDeviceManager.d \
./src/CDeviceMonitor.d \
./src/CListener.d \
./src/CSocketManager.d \
./src/CommandTranslater.d \
./src/ProxyLogger.d \
./src/ReplyFactory.d \
./src/ReplyTranslater.d \
./src/proxy.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/mikez/Poco/poco-1.9.0/include -I"/home/mikez/Developments/invenco/crc/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


