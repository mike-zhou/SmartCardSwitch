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
	static std::shared_ptr<Command> BdcsPowerQueryPower();
	static std::shared_ptr<Command> BdcOperation(unsigned int bdcIndex,
												CommandBdcOperation::BdcMode initialMode,
												CommandBdcOperation::BdcMode finalMode,
												unsigned long delayMs);
	static std::shared_ptr<Command> StepperQueryClkPeriod();
};



#endif /* COMMANDFACTORY_H_ */
