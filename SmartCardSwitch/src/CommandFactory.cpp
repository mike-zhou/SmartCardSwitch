/*
 * CommandFactory.cpp
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#include "CommandFactory.h"
#include "MsgPackager.h"

std::shared_ptr<Command> CommandFactory::DevicesGet()
{
	std::shared_ptr<Command> ptr(new CommandDevicesGet());

	return ptr;
}

std::shared_ptr<Command> CommandFactory::DeviceConnect(const std::string& deviceName)
{
	std::shared_ptr<Command> ptr(new CommandDeviceConnect(deviceName));

	return ptr;
}

std::shared_ptr<Command> CommandFactory::DeviceQueryPower()
{
	std::shared_ptr<Command> ptr(new CommandDeviceQueryPower());

	return ptr;
}

std::shared_ptr<Command> CommandFactory::BdcsPowerOn()
{
	std::shared_ptr<Command> ptr(new CommandBdcsPowerOn());

	return ptr;
}


std::shared_ptr<Command> CommandFactory::BdcsPowerOff()
{
	std::shared_ptr<Command> ptr(new CommandBdcsPowerOff());

	return ptr;
}


std::shared_ptr<Command> CommandFactory::BdcsQueryPower()
{
	std::shared_ptr<Command> ptr(new CommandBdcsQueryPower());

	return ptr;
}


std::shared_ptr<Command> CommandFactory::BdcOperation(unsigned int bdcIndex,
											CommandBdcOperation::BdcMode initialMode,
											CommandBdcOperation::BdcMode finalMode)
{
	std::shared_ptr<Command> ptr(new CommandBdcOperation(bdcIndex, initialMode, finalMode));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperQueryClkPeriod()
{
	std::shared_ptr<Command> ptr(new CommandStepperQueryClkPeriod());

	return ptr;
}

std::shared_ptr<Command> CommandFactory::StepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks)
{
	std::shared_ptr<Command> ptr(new CommandStepperConfigStep(stepperIndex, lowClks, highClks));

	return ptr;
}

std::shared_ptr<Command> CommandFactory::StepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<Command> ptr(new CommandStepperAccelerationBuffer(stepperIndex, value));

	return ptr;
}

std::shared_ptr<Command> CommandFactory::StepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<Command> ptr(new CommandStepperAccelerationBufferDecrement(stepperIndex, value));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<Command> ptr(new CommandStepperDecelerationBuffer(stepperIndex, value));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value)
{
	std::shared_ptr<Command> ptr(new CommandStepperDecelerationBufferIncrement(stepperIndex, value));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperEnable(unsigned int stepperIndex, bool enable)
{
	std::shared_ptr<Command> ptr(new CommandStepperEnable(stepperIndex, enable));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperConfigHome(unsigned int stepperIndex, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal)
{
	std::shared_ptr<Command> ptr(new CommandStepperConfigHome(stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal));

	return ptr;
}


std::shared_ptr<Command> CommandFactory::StepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps)
{
	std::shared_ptr<Command> ptr(new CommandStepperMove(stepperIndex, position, forward, steps));

	return ptr;
}



