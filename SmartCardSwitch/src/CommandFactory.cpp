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

std::shared_ptr<DeviceCommand> CommandFactory::DeviceDelay(unsigned int clks)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDeviceDelay(clks));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DeviceQueryPower()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDeviceQueryPower());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DeviceQueryFuse()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDeviceQueryFuse());

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
											CommandBdcOperation::BdcMode finalMode,
											unsigned int lowClks,
											unsigned int highClks,
											unsigned int cycles)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcOperation(bdcIndex, initialMode, finalMode, lowClks, highClks, cycles));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::BdcQuery(unsigned int bdcIndex)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandBdcQuery(bdcIndex));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperQueryResolution()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperQueryResolution());

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

std::shared_ptr<DeviceCommand> CommandFactory::SteppersPowerOn()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandSteppersPowerOn);

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::SteppersPowerOff()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandSteppersPowerOff);

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::SteppersQueryPower()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandSteppersQueryPower);

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperEnable(unsigned int stepperIndex, bool enable)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperEnable(stepperIndex, enable));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperForward(unsigned int stepperIndex, bool forward)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperForward(stepperIndex, forward));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperSteps(unsigned int stepperIndex, unsigned long steps)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperSteps(stepperIndex, steps));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperRun(unsigned int stepperIndex, unsigned long initialPosition, unsigned long finalPosition)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperRun(stepperIndex, initialPosition, finalPosition));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperConfigHome(unsigned int stepperIndex, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperConfigHome(stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperQuery(unsigned int stepperIndex)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperQuery(stepperIndex));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperSetState(unsigned int stepperIndex, unsigned int state)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperSetState(stepperIndex, state));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::StepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperMove(stepperIndex, position, forward, steps));

	return ptr;
}
std::shared_ptr<DeviceCommand> CommandFactory::StepperForwardClockwise(unsigned int stepperIndex, bool forwardClockwise)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandStepperForwardClockwise(stepperIndex, forwardClockwise));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::LocatorQuery(unsigned int locatorIndex)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandLocatorQuery(locatorIndex));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::OptPowerOn()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandOptPowerOn());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::OptPowerOff()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandOptPowerOff());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::OptQueryPower()
{
	std::shared_ptr<DeviceCommand> ptr(new CommandOptQueryPower());

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DcmPowerOn(unsigned int index)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDcmPowerOn(index));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DcmPowerOff(unsigned int index)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDcmPowerOff(index));

	return ptr;
}

std::shared_ptr<DeviceCommand> CommandFactory::DcmQueryPower(unsigned int index)
{
	std::shared_ptr<DeviceCommand> ptr(new CommandDcmQueryPower(index));

	return ptr;
}

