/*
 * ReplyFactory.cpp
 *
 *  Created on: Oct 10, 2018
 *      Author: user1
 */

#include "ReplyFactory.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

void ReplyFactory::createReply(const std::string& reply, std::vector<unsigned char>& result)
{
	unsigned long length = reply.size() + versionWidth + lengthWidth;

	pLogger->LogDebug("ReplyFactory::createReply packaging: " + reply);

	//********* little endian ************
	//header, 2 bytes
	result.push_back((HEADER_TAG >> 8) & 0xff);
	result.push_back(HEADER_TAG & 0xff);
	//length, 2 bytes
	result.push_back((length >> 8) & 0xff);
	result.push_back(length & 0xff);
	//version
	result.push_back((VERSION >> 8) & 0xff);
	result.push_back((VERSION) & 0xff);
	//JSON
	for(auto it = reply.begin(); it != reply.end(); it++) {
		result.push_back(*it);
	}
	//tail, 2 bytes
	result.push_back((TAIL_TAG >> 8) & 0xff);
	result.push_back(TAIL_TAG & 0xff);
}

//return the package containing a JSON event.
std::vector<unsigned char> ReplyFactory::EventDeviceUnplugged()
{
	std::vector<unsigned char> vector;
	std::string reply = "{\"event\":\"device is unplugged\"}";

	createReply(reply, vector);
	return vector;
}

// return the package containing the JSON reply.
std::vector<unsigned char> ReplyFactory::Reply(const std::string& reply)
{
	std::vector<unsigned char> vector;

	createReply(reply, vector);
	return vector;
}

//return the pakcage containing a JSON array of devices
std::vector<unsigned char> ReplyFactory::DevicesGet(const std::vector<std::string>& devices)
{
	std::vector<unsigned char> vector;
	std::string json;

	//{
	//	"command":"devices get",
	//	"devices":["device1", "device2"]
	//}

	json = "{";
	json += "\"command\":\"devices get\",";
	json += "\"devices\":[";
	for(auto it=devices.begin(); it!=devices.end(); it++) {
		json += "\"" + *it + "\",";
	}
	if(devices.size() > 0) {
		json.pop_back();//remove the last ','
	}
	json += "]";
	json += "}";

	createReply(json, vector);

	return vector;
}

std::vector<unsigned char> ReplyFactory::DeviceConnect(const std::string& deviceName, bool result, const std::string& reason)
{
	std::vector<unsigned char> vector;
	std::string json;

	//{
	//	"command":"device connect",
	//	"device": "device12345",
	//	"result":false,
	//	"reason":"connected to IP:port"
	//}

	json = "{";
	json = json + "\"command\":\"device connect\",";
	json = json + "\"device\":\"" + deviceName + "\",";
	json = json + "\"result\":" + (result?"true":"false") + ",";
	json = json + "\"reason\":\"" + reason + "\"";
	json = json + "}";

	createReply(json, vector);

	return vector;
}
