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
	//return command package of devices get.
	static std::shared_ptr<Command> DevicesGet();

	//return command package of device connect.
	static std::shared_ptr<Command> DeviceConnect(const std::string& deviceName);
};



#endif /* COMMANDFACTORY_H_ */
