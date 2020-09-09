################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ClientListener.cpp \
../src/LinuxRS232.cpp \
../src/SocketTransceiver.cpp \
../src/rs232eth.cpp 

OBJS += \
./src/ClientListener.o \
./src/LinuxRS232.o \
./src/SocketTransceiver.o \
./src/rs232eth.o 

CPP_DEPS += \
./src/ClientListener.d \
./src/LinuxRS232.d \
./src/SocketTransceiver.d \
./src/rs232eth.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/user1/Poco/poco-1.9.0/include -I"/home/user1/Developments/invenco/Logger/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


