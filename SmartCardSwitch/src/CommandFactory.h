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
	static std::shared_ptr<Command> DevicesGet();
	static std::shared_ptr<Command> DeviceConnect(const std::string& deviceName);
	static std::shared_ptr<Command> DeviceQueryPower();
	static std::shared_ptr<Command> BdcsPowerOn();
	static std::shared_ptr<Command> BdcsPowerOff();
	static std::shared_ptr<Command> BdcsQueryPower();
	static std::shared_ptr<Command> BdcOperation(unsigned int bdcIndex,
												CommandBdcOperation::BdcMode initialMode,
												CommandBdcOperation::BdcMode finalMode,
												unsigned long delayMs);
	static std::shared_ptr<Command> StepperQueryClkPeriod();
	static std::shared_ptr<Command> StepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks);
	static std::shared_ptr<Command> StepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<Command> StepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<Command> StepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<Command> StepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value);
	static std::shared_ptr<Command> StepperEnable(unsigned int stepperIndex, bool enable);
	static std::shared_ptr<Command> StepperConfigHome(unsigned int stepperIndex, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal);
	static std::shared_ptr<Command> StepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps);
};



#endif /* COMMANDFACTORY_H_ */
