/*
 * CommandFactory.cpp
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#include "CommandFactory.h"
#include "MsgPackager.h"

std::shared_ptr<DeviceCommand> CommandFactory::DevicesGet()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDevicesGet());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DeviceConnect(const std::string& deviceName)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDeviceConnect(deviceName));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DeviceQueryPower()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDeviceQueryPower());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::BdcsPowerOn()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcsPowerOn());

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::BdcsPowerOff()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcsPowerOff());

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::BdcsQueryPower()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcsQueryPower());

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::BdcOperation(unsigned int bdcIndex,
											CommandBdcOperation::BdcMode initialMode,
											CommandBdcOperation::BdcMode finalMode)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcOperation(bdcIndex, initialMode, finalMode));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperQueryClkPeriod()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperQueryClkPeriod());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperConfigStep(stepperIndex, lowClks, highClks));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperAccelerationBuffer(stepperIndex, value));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperAccelerationBufferDecrement(stepperIndex, value));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperDecelerationBuffer(stepperIndex, value));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperDecelerationBufferIncrement(stepperIndex, value));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperEnable(unsigned int stepperIndex, bool enable)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperEnable(stepperIndex, enable));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperConfigHome(unsigned int stepperIndex, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperConfigHome(stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal));

	return ptr;
}


std::shared_ptr<DeviceCommand> CommandFactory::StepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperMove(stepperIndex, position, forward, steps));

	return ptr;
}



