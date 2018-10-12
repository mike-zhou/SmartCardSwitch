/*
 * CommandFactory.h
 *
 *  Created on: Oct 12, 2018
 *      Author: user1
 */

#ifndef COMMANDFACTORY_H_
#define COMMANDFACTORY_H_

#include <string>
#include <vector>

class CommandFactory
{
	// Format of command package: HEADER_TAG length version jsonCommand TAIL_TAG
public:
	//return command package of devices get.
	static std::vector<unsigned char> DevicesGet();

	//return command package of device connect.
	static std::vector<unsigned char> DeviceConnect(const std::string& deviceName);

private:
	static const unsigned short HEADER_TAG = 0xAABB;
	static const unsigned short VERSION = 0x0000;
	static const unsigned short TAIL_TAG = 0xCCDD;

	static const int headerWidth = 2;
	static const int lengthWidth= 2;
	static const int versionWidth = 2;
	static const int tailWidth = 2;

	//put the jsonCmd to a package of TagLengthValue.
	static void packageJsonCmd(const std::string& jsonCmd /*input*/, std::vector<unsigned char>& result/*output*/);
};



#endif /* COMMANDFACTORY_H_ */
