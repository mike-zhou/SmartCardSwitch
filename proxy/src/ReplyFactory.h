/*
 * ReplyFactory.h
 *
 *  Created on: Oct 6, 2018
 *      Author: user1
 */

#ifndef REPLYFACTORY_H_
#define REPLYFACTORY_H_

#include <vector>
#include <string>

class ReplyFactory
{
	//format of reply message:
	// HEADER_TAG length version JSON TAIL_TAG

public:
	//return the package containing a JSON event.
	static std::vector<unsigned char> EventDeviceUnplugged();

	// return the package containing the JSON reply.
	static std::vector<unsigned char> Reply(const std::string& reply);

	//return the pakcage containing a JSON array of devices
	static std::vector<unsigned char> DevicesGet(const std::vector<std::string>& devices);

	static std::vector<unsigned char> DeviceConnect(const std::string& deviceName, bool result, const std::string& reason);

private:
	static const unsigned short HEADER_TAG = 0xAABB;
	static const unsigned short VERSION = 0x0000;
	static const unsigned short TAIL_TAG = 0xCCDD;

	static const int versionWidth = 2;
	static const int lengthWidth = 2;

	static void createReply(const std::string& reply, std::vector<unsigned char>& result);
};



#endif /* REPLYFACTORY_H_ */
