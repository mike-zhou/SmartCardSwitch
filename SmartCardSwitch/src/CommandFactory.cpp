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
											CommandBdcOperation::BdcMode finalMode,
											unsigned long delayMs)
{
	std::shared_ptr<Command> ptr(new CommandBdcOperation(bdcIndex, initialMode, finalMode, delayMs));

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


