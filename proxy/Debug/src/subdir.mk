################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CDataExchange.cpp \
../src/CDeviceManager.cpp \
../src/CDeviceMonitor.cpp \
../src/CListener.cpp \
../src/CSocketManager.cpp \
../src/CommandTranslater.cpp \
../src/LinuxComDevice.cpp \
../src/ProxyLogger.cpp \
../src/ReplyFactory.cpp \
../src/ReplyTranslater.cpp \
../src/WinComDevice.cpp \
../src/proxy.cpp 

OBJS += \
./src/CDataExchange.o \
./src/CDeviceManager.o \
./src/CDeviceMonitor.o \
./src/CListener.o \
./src/CSocketManager.o \
./src/CommandTranslater.o \
./src/LinuxComDevice.o \
./src/ProxyLogger.o \
./src/ReplyFactory.o \
./src/ReplyTranslater.o \
./src/WinComDevice.o \
./src/proxy.o 

CPP_DEPS += \
./src/CDataExchange.d \
./src/CDeviceManager.d \
./src/CDeviceMonitor.d \
./src/CListener.d \
./src/CSocketManager.d \
./src/CommandTranslater.d \
./src/LinuxComDevice.d \
./src/ProxyLogger.d \
./src/ReplyFactory.d \
./src/ReplyTranslater.d \
./src/WinComDevice.d \
./src/proxy.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/mikez/Poco/poco-1.9.0/include -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


