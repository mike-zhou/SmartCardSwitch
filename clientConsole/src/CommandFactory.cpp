/*
 * CommandFactory.cpp
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#include "CommandFactory.h"

void CommandFactory::packageJsonCmd(const std::string& jsonCmd /*input*/, std::vector<unsigned char>& result/*output*/)
{
	unsigned short length;

	printf("CommandFactory::packageJsonCmd %s\r\n", jsonCmd.c_str());

	//TAG
	result.push_back((HEADER_TAG >> 8) & 0xff);
	result.push_back(HEADER_TAG & 0xFF);

	//Length
	length = versionWidth + jsonCmd.size() + tailWidth;
	result.push_back((length >> 8) & 0xff);
	result.push_back(length & 0xff);

	//value
	//value.version
	result.push_back((VERSION >> 8) & 0xff);
	result.push_back(VERSION & 0xff);
	//value.cmd
	for(auto it=jsonCmd.begin(); it!=jsonCmd.end(); it++) {
		result.push_back(*it);
	}
	//value.tail
	result.push_back((TAIL_TAG >> 8) & 0xff);
	result.push_back(TAIL_TAG & 0xff);
}

std::vector<unsigned char> CommandFactory::DevicesGet()
{
	std::string jsonCmd;
	std::vector<unsigned char> cmdPkg;

	jsonCmd = "{";
	jsonCmd = jsonCmd + "\"command\":\"devices get\"";
	jsonCmd += "}";

	packageJsonCmd(jsonCmd, cmdPkg);

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

	packageJsonCmd(jsonCmd, cmdPkg);

	return cmdPkg;
}

