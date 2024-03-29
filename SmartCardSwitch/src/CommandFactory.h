/*
 * CommandFactory.h
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#ifndef COMMANDFACTORY_H_
#define COMMANDFACTORY_H_

#include <string>
#include <memory>
#include "Command.h"

class CommandFactory
{
	// Format of command package: HEADER_TAG length version jsonCommand TAIL_TAG
public:
	static std::shared_ptr<DeviceCommand> DevicesGet();
	static std::shared_ptr<DeviceCommand> DeviceConnect(const std::string& deviceName);
	static std::shared_ptr<DeviceCommand> DeviceDelay(unsigned int clks);
	static std::shared_ptr<DeviceCommand> DeviceQueryPower();
	static std::shared_ptr<DeviceCommand> DeviceQueryFuse();
	static std::shared_ptr<DeviceCommand> BdcsPowerOn();
	static std::shared_ptr<DeviceCommand> BdcsPowerOff();
	static std::shared_ptr<DeviceCommand> BdcsQueryPower();
	static std::shared_ptr<DeviceCommand> BdcOperation(unsigned int bdcIndex,
														CommandBdcOperation::BdcMode initialMode,
														CommandBdcOperation::BdcMode finalMode,
														unsigned int lowClks,
														unsigned int highClks,
														unsigned int cycles);

	static std::shared_ptr<DeviceCommand> BdcQuery(unsigned int bdcIndex);
	static std::shared_ptr<DeviceCommand> StepperQueryResolution();
	static std::shared_ptr<DeviceCommand> StepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks);
	static std::shared_ptr<DeviceCommand> StepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<DeviceCommand> StepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<DeviceCommand> StepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<DeviceCommand> StepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<DeviceCommand> SteppersPowerOn();
	static std::shared_ptr<DeviceCommand> SteppersPowerOff();
	static std::shared_ptr<DeviceCommand> SteppersQueryPower();
	static std::shared_ptr<DeviceCommand> StepperEnable(unsigned int stepperIndex, bool enable);
	static std::shared_ptr<DeviceCommand> StepperForward(unsigned int stepperIndex, bool forward);
	static std::shared_ptr<DeviceCommand> StepperSteps(unsigned int stepperIndex, unsigned long steps);
	static std::shared_ptr<DeviceCommand> StepperRun(unsigned int stepperIndex, unsigned long initialPosition, unsigned long finalPosition);
	static std::shared_ptr<DeviceCommand> StepperConfigHome(unsigned int stepperIndex, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal);
	static std::shared_ptr<DeviceCommand> StepperQuery(unsigned int stepperIndex);
	static std::shared_ptr<DeviceCommand> StepperSetState(unsigned int stepperIndex, unsigned int state);
	static std::shared_ptr<DeviceCommand> StepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps);
	static std::shared_ptr<DeviceCommand> StepperForwardClockwise(unsigned int stepperIndex, bool forwardClockwise);
	static std::shared_ptr<DeviceCommand> LocatorQuery(unsigned int locatorIndex);
	static std::shared_ptr<DeviceCommand> OptPowerOn();
	static std::shared_ptr<DeviceCommand> OptPowerOff();
	static std::shared_ptr<DeviceCommand> OptQueryPower();
	static std::shared_ptr<DeviceCommand> DcmPowerOn(unsigned int index);
	static std::shared_ptr<DeviceCommand> DcmPowerOff(unsigned int index);
	static std::shared_ptr<DeviceCommand> DcmQueryPower(unsigned int index);
};



#endif /* COMMANDFACTORY_H_ */
