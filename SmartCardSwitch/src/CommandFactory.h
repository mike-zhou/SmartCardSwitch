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
};



#endif /* COMMANDFACTORY_H_ */
