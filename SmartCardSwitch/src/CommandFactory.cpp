/*
 * CommandFactory.cpp
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#include "CommandFactory.h"
#include "MsgPackager.h"

std::vector<unsigned char> CommandFactory::DevicesGet()
{
	std::string jsonCmd;
	std::vector<unsigned char> cmdPkg;

	jsonCmd = "{";
	jsonCmd = jsonCmd + "\"command\":\"devices get\"";
	jsonCmd += "}";

	MsgPackager::PackageMsg(jsonCmd, cmdPkg);

	return cmdPkg;
}

std::vector<unsigned char> CommandFactory::DeviceConnect(const std::string& deviceName)
{
	std::string jsonCmd;
	std::vector<unsigned char> cmdPkg;

	jsonCmd = "{";
	jsonCmd = jsonCmd + "\"command\":\"device connect\",";
	jsonCmd = jsonCmd + "\"device\":\"" + deviceName + "\"";
	jsonCmd += "}";

	MsgPackager::PackageMsg(jsonCmd, cmdPkg);

	return cmdPkg;
}

