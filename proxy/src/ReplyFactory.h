/*
 * ReplyFactory.h
 *
 *  Created on: Oct 6, 2018
 *      Author: user1
 */

#ifndef REPLYFACTORY_H_
#define REPLYFACTORY_H_

#include <vector>

class ReplyFactory
{
	//format of reply message:
	// HEADER_TAG length version JSON TAIL_TAG
public:
	static std::vector<unsigned char> EventDeviceUnplugged()
	{
		std::vector<unsigned char> vector;
		std::string reply = "{\"event\":\"device is unplugged\"}";

		createReply(reply, vector);
		return vector;
	}

	static std::vector<unsigned char> Reply(const std::string& reply)
	{
		std::vector<unsigned char> vector;

		createReply(reply, vector);
		return vector;
	}

private:
	static const unsigned short HEADER_TAG = 0xAABB;
	static const unsigned short VERSION = 0x0000;
	static const unsigned short TAIL_TAG = 0xCCDD;

	static const int lengthOfVersion = 2;
	static const int lengthOfTail = 2;

	static void createReply(const std::string& reply, std::vector<unsigned char>& result)
	{
		unsigned long length = reply.size() + lengthOfVersion + lengthOfTail;

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
};



#endif /* REPLYFACTORY_H_ */
