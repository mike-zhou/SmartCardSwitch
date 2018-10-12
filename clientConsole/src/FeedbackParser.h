/*
 * FeedbackParser.h
 *
 *  Created on: Oct 13, 2018
 *      Author: user1
 */

#ifndef FEEDBACKPARSER_H_
#define FEEDBACKPARSER_H_

#include <vector>
#include <deque>

class FeedbackParser
{
private:
	static const unsigned short HEADER_TAG = 0xAABB;
	static const unsigned short VERSION = 0x0000;
	static const unsigned short TAIL_TAG = 0xCCDD;

	static const int headerWidth = 2;
	static const int lengthWidth= 2;
	static const int versionWidth = 2;
	static const int tailWidth = 2;
	static const int minimumCommandPacketLength = headerWidth + lengthWidth + versionWidth + tailWidth;

	enum DataState
	{
		HEADER_ERROR,
		LENGTH_ERROR,
		VERSION_ERROR,
		JSON_ERROR,
		TAIL_ERROR,
		PARTIAL_COMMAND,
		COMPLETE_COMMAND
	};

	//retrieve a command from deque and delete bytes of a complete command package
	static DataState retrieveJson(std::deque<unsigned char>& data, std::string& json)
	{
		unsigned short contentLength;
		unsigned short version;

		if(data.size() < minimumCommandPacketLength) {
			return DataState::PARTIAL_COMMAND;
		}
		//check header
		if(data[0] != ((HEADER_TAG>>8)&0xff)) {
			return DataState::HEADER_ERROR;
		}
		if(data[1] != (HEADER_TAG&0xff)) {
			return DataState::HEADER_ERROR;
		}
		//check length
		contentLength = data[2];
		contentLength = (contentLength<<8)+data[3];
		if(contentLength < (versionWidth + tailWidth)) {
			return DataState::LENGTH_ERROR;
		}
		//check version
		version = data[4];
		version = (version<<8) + data[5];
		if(version > VERSION) {
			return DataState::VERSION_ERROR;
		}
		//check package length
		if((contentLength + headerWidth + lengthWidth) > data.size()) {
			return DataState::PARTIAL_COMMAND;
		}
		//check tail
		if(data[headerWidth + lengthWidth + contentLength -2] != ((TAIL_TAG >> 8) & 0xff)) {
			return DataState::TAIL_ERROR;
		}
		if(data[headerWidth + lengthWidth + contentLength -1] != ((TAIL_TAG) & 0xff)) {
			return DataState::TAIL_ERROR;
		}
		//check JSON
		for(int i=0; i<(contentLength - versionWidth - tailWidth); i++) {
			unsigned char c = data[headerWidth + lengthWidth + versionWidth +i];
			if((c < ' ') || (c > '~')) {
				//illegal JSON character
				return DataState::JSON_ERROR;
			}
		}
		//retrieve command
		for(int i=0; i<(contentLength - versionWidth - tailWidth); i++) {
			unsigned char c = data[headerWidth + lengthWidth + versionWidth +i];
			json.push_back(c);
		}
		//delete data
		for(int i=0; i<(headerWidth + lengthWidth + contentLength); i++) {
			data.pop_front();
		}

		return DataState::COMPLETE_COMMAND;
	}

public:
	// make JSON command with data.
	// invalid data at the beginning of data will be deleted.
	// if a JSON command is created, the relevant content in data is deleted.
	static void RetrieveFeedbacks(std::deque<unsigned char>& data/*input*/, std::vector<std::string>& jsons/*output*/)
	{
		for(;;)
		{
			std::string json;

			auto rc = retrieveJson(data, json);

			if((rc == DataState::HEADER_ERROR) ||
				(rc == DataState::LENGTH_ERROR) ||
				(rc == DataState::VERSION_ERROR) ||
				(rc == DataState::JSON_ERROR) ||
				(rc == DataState::TAIL_ERROR))
			{
				//discard the first byte
				data.pop_front();
			}
			else if(rc == DataState::PARTIAL_COMMAND)
			{
				break;
			}
			else if(rc == DataState::COMPLETE_COMMAND)
			{
				jsons.push_back(json);
			}
		}
	}
};




#endif /* FEEDBACKPARSER_H_ */
