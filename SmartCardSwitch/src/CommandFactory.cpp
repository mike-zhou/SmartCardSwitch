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

