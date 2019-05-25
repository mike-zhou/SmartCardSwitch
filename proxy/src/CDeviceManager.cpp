/*
 * CDeviceManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */
#include "stddef.h"
#include "fcntl.h"
#include "CDeviceManager.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/Format.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "ProxyLogger.h"
#include <poll.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

using Poco::DirectoryIterator;
using Poco::File;
using Poco::Path;

extern ProxyLogger * pLogger;

static void UcharToHex(unsigned char c, unsigned char & low4Bits, unsigned char & highBits)
{
	unsigned char h4 = (c >> 4) & 0x0F;
	unsigned char l4 = c & 0x0F;

	if(l4 <= 9) {
		low4Bits = l4 + '0';
	}
	else {
		low4Bits = l4 - 0xA + 'A';
	}

	if(h4 <= 9) {
		highBits = h4 + '0';
	}
	else {
		highBits = h4 - 0xA + 'A';
	}
}

static unsigned char UcharFromHex(unsigned char low4Bits, unsigned char high4Bits)
{
	unsigned char c = 0;

	if((low4Bits >= '0') && (low4Bits <= '9')) {
		c = low4Bits - '0';
	}
	else if((low4Bits >= 'A') && (low4Bits <= 'F')){
		c = low4Bits - 'A' + 0xA;
	}
	else if((low4Bits >= 'a') && (low4Bits <= 'f')){
		c = low4Bits - 'a' + 0xA;
	}
	else {
		pLogger->LogError("UcharFromHex wrong low4Bits: " + std::to_string(low4Bits));
	}

	if((high4Bits >= '0') && (high4Bits <= '9')) {
		c += (high4Bits - '0') << 4;
	}
	else if((high4Bits >= 'A') && (high4Bits <= 'F')){
		c += (high4Bits - 'A' + 0xA) << 4;
	}
	else if((high4Bits >= 'a') && (high4Bits <= 'f')){
		c += (high4Bits - 'a' + 0xA) << 4;
	}
	else {
		pLogger->LogError("UcharFromHex wrong high4Bits: " + std::to_string(high4Bits));
	}

	return c;
}


CDeviceManager::CDeviceManager() : Task("CDeviceManager")
{
	_pObserver = NULL;
	_startMonitoringDevices = false;
}

CDeviceManager::~CDeviceManager() {
	// TODO Auto-generated destructor stub
}

void CDeviceManager::SetObserver(IDeviceObserver * pObserver)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_pObserver = pObserver;
}

void CDeviceManager::StartMonitoringDevices()
{
	_startMonitoringDevices = true;
}

void CDeviceManager::SendCommand(const std::string& deviceName, const std::string& command)
{
	for(auto it=command.begin(); it!=command.end(); it++) {
		if((*it < ' ') || (*it > '~')) {
			pLogger->LogError("CDeviceManager::SendCommand illegal character in command: " + deviceName + ":" + command);
			return;
		}
	}

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _devices.begin();
	for(; it != _devices.end(); it++) {
		if(it->state == DeviceState::ACTIVE) {
			if(it->deviceName == deviceName) {
				pLogger->LogDebug("CDeviceManager enqueue command: " + deviceName + ":" + command);
				enqueueCommand(*it, command);
				break;
			}
		}
	}
	if(it == _devices.end()) {
		pLogger->LogError("CDeviceManager::SendCommand failed in sending: " + deviceName + ":" + command);
	}
}

void CDeviceManager::AddDeviceFile(const std::string & deviceFilePath)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	bool fileExist = false;

	for(auto it = _deviceFiles.begin(); it != _deviceFiles.end(); it++)
	{
		if(*it == deviceFilePath) {
			fileExist = true;
			break;
		}
	}

	if(!fileExist) {
		_deviceFiles.push_back(deviceFilePath);
	}
}

void CDeviceManager::checkDevices()
{
	//check devices every 1 second
	static Poco::Timestamp timeStamp;
	if(timeStamp.elapsed() <= 1000000) {
		return;
	}
	else {
		timeStamp.update();
	}

	std::vector<std::string> deviceUnpluged;
	std::vector<std::string> currentOpenedFiles;

	try
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);

		if(_deviceFiles.empty()) {
			pLogger->LogError("CDeviceManager::checkDevices no controlling file is specified");
		}
		else
		{
			for(auto it = _deviceFiles.begin(); it != _deviceFiles.end(); it++)
			{
				bool fileIsOpened = false;
				Path p(*it);
				int fd;

				//check if current device has been opened
				for(size_t i = 0; i<_devices.size(); i++) {
					if(_devices[i].fileName == p.getFileName()) {
						fileIsOpened = true;
						break;
					}
				}
				if(fileIsOpened) {
					currentOpenedFiles.push_back(p.getFileName());
					continue;
				}

				std::string fileName = *it;;
				//open device
				fd = open(fileName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
				if(fd < 0){
					auto errorNumber = errno;
					//cannot open device file
					pLogger->LogError("CDeviceManager::checkDevices cannot open: " + fileName + ", reason: " + std::string(strerror(errorNumber)));
					continue;
				}

				//set device file
				{
					struct termios tios;
					int rc;
					//change settings of device.
					rc = tcgetattr(fd, &tios);
					if(0 != rc)
					{
						auto e = errno;
						pLogger->LogError("CDeviceManager::checkDevices tcgetattr errno: " + std::to_string(e));
						close(fd);
						continue;
					}
					rc = cfsetspeed(&tios, B115200);
					if(0 != rc)
					{
						auto e = errno;
						pLogger->LogError("CDeviceManager::checkDevices cfsetspeed errno: " + std::to_string(e));
						close(fd);
						continue;
					}
#if 0
					//c_iflag
					tios.c_iflag &= ~ICRNL;
					tios.c_iflag &= ~IXON;
					tios.c_iflag |= IGNPAR;
					//c_oflag
					tios.c_oflag &= ~ONLCR;
					tios.c_oflag &= ~OPOST;
					//c_cflag
					tios.c_cflag &= ~CLOCAL;
//					tios.c_cflag |= CS8;
//					tios.c_cflag &= ~CSTOPB;
//					tios.c_cflag &= ~PARENB;
					//c_lflag
					tios.c_lflag &= ~ISIG;
					tios.c_lflag &= ~ICANON;
					tios.c_lflag &= ~IEXTEN;
					tios.c_lflag &= ~ECHO;
					tios.c_lflag &= ~ECHOE;
					tios.c_lflag &= ~ECHOK;
					tios.c_lflag &= ~ECHOCTL;
					tios.c_lflag &= ~ECHOKE;
					tios.c_lflag &= ~FLUSHO;
					tios.c_lflag &= ~EXTPROC;
#endif

#if 1 //cfmakeraw
					cfmakeraw(&tios);
					//polling read.
					tios.c_cc[VMIN] = 0;
					tios.c_cc[VTIME] = 0;
					//8N1
					tios.c_cflag |= CS8;
					tios.c_cflag &= ~CSTOPB; //1 stop bit
					tios.c_cflag &= ~PARENB; //no parity
					//others
					tios.c_cflag |= CLOCAL; //ignore modem control lines
					tios.c_cflag |= CREAD; //enable receiver
					tios.c_cflag &= ~CRTSCTS; //no RTS/CTS flow control
#endif
					rc = tcsetattr(fd, TCSANOW, &tios);
					if(0 != rc)
					{
						auto e = errno;
						pLogger->LogError("CDeviceManager::checkDevices tcsetattr errno: " + std::to_string(e));
						close(fd);
						continue;
					}

					//remember this new device.
					Poco::ScopedLock<Poco::Mutex> lock(_mutex);
					struct Device device;

					device.fd = fd;
					device.fileName = p.getFileName();
					device.state = DeviceState::OPENED;
					_devices.push_back(device);
				}

				currentOpenedFiles.push_back(p.getFileName());
				pLogger->LogInfo("CDeviceManager::checkDevices file is opened: " + p.getFileName());
			}
		}
	}
	catch (Poco::Exception& exc)
	{
		pLogger->LogError("CDeviceManager::checkDevices exception: " + exc.displayText());
	}
	catch(...)
	{
		pLogger->LogError("CDeviceManager::checkDevices unknown exception");
	}

	//check if any device was unplugged.
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);

		for(auto oldDeviceIt = _devices.begin(); oldDeviceIt!=_devices.end(); )
		{
			auto newDeviceIt = currentOpenedFiles.begin();
			for(; newDeviceIt!=currentOpenedFiles.end(); newDeviceIt++)
			{
				if(oldDeviceIt->fileName == *newDeviceIt) {
					break;
				}
			}
			if(newDeviceIt == currentOpenedFiles.end()) {
				//the old device is NOT found in the new file list
				pLogger->LogInfo("CDeviceManager::checkDevices device is unplugged: " + oldDeviceIt->fileName);
				if(oldDeviceIt->state == DeviceState::ACTIVE) {
					//observers are to be notified
					deviceUnpluged.push_back(oldDeviceIt->deviceName); // deviceName rather than fileName.
				}
				//delete the old device
				oldDeviceIt = _devices.erase(oldDeviceIt);
			}
			else {
				oldDeviceIt++;
			}
		}
	}

	//outside mutex protection to avoid dead lock.
	if(deviceUnpluged.size() > 0) {
		for(auto it = deviceUnpluged.begin(); it != deviceUnpluged.end(); it++) {
			_pObserver->OnDeviceUnplugged(*it);
		}
	}
}

void CDeviceManager::onReply(struct Device& device, const std::string& reply)
{
	//convert the reply to a JSON object.
	std::string json = "{" + reply + "}";

	switch(device.state)
	{
		case DeviceState::RECEIVING_NAME:
		{
			if(reply != std::string(COMMAND_QUERY_NAME))
			{
				bool exceptionOccur = false;
				bool nameAvailable = false;
				std::string name;

				try
				{
					Poco::JSON::Parser parser;
					Poco::Dynamic::Var result = parser.parse(json);
					Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

					if(objectPtr->has(std::string("name")))
					{
						name = objectPtr->getValue<std::string>("name");
						if(name.size() < 1) {
							pLogger->LogError("CDeviceManager::onReply invalid name in " + reply);
						}
						else {
							nameAvailable = true;
						}
					}
					else
					{
						pLogger->LogError("CDeviceManager::onReply no name in " + reply);
					}
				}
				catch(Poco::JSON::JSONException& e)
				{
					exceptionOccur = true;
					pLogger->LogError("CDeviceManager::onReply exception occurs: " + e.displayText() + " : " + json);
				}
				catch(...)
				{
					exceptionOccur = true;
					pLogger->LogError("CDeviceManager::onReply unknown exception in " + json);
				}

				if(!exceptionOccur && nameAvailable)
				{
					device.deviceName = name;
					device.state = DeviceState::ACTIVE;
					pLogger->LogInfo("CDeviceManager::onReply device inserted: " + device.deviceName + ":" + device.fileName);
					_pObserver->OnDeviceInserted(device.deviceName);
				}
			}
		}
		break;

		case DeviceState::ACTIVE:
		{
			_pObserver->OnDeviceReply(device.deviceName, json);
		}
		break;

		default:
		{
			//do nothing
		}
		break;
	}
}

//read data from device
void CDeviceManager::onDeviceCanBeRead(struct Device& device)
{
	std::string binaryLog;
	std::string charLog;
	std::vector<char> appData;

	{
		auto errorNumber = errno;
		auto & stage = device.dataExchange.inputStage;

		if(stage.byteAmount > 0)
		{
			//a partial packet is in buffer
			if(stage.inputTimeStamp.elapsed() >= DATA_INPUT_TIMEOUT)
			{
				pLogger->LogError("CDeviceManager::onDeviceCanBeRead packet receiving timeout, discard " + std::to_string(stage.byteAmount) + " bytes");
				stage.byteAmount = 0; //discard the partial packet
			}
		}

		auto amount = read(device.fd, stage.buffer + stage.byteAmount, PACKET_SIZE - stage.byteAmount);
		errorNumber = errno;
		if(amount == 0) {
			pLogger->LogError("CDeviceManager::onDeviceCanBeRead nothing is read, errno: " + std::to_string(errorNumber));
		}
		else if(amount < 0) {
			pLogger->LogError("CDeviceManager::onDeviceCanBeRead error in device, errno: " + std::to_string(errorNumber));
		}
		else
		{
			//some bytes arrive.
			{
				char log[256];
				sprintf(log, "CDeviceManager::onDeviceCanBeRead %ld bytes from %s", amount, device.fileName.c_str());
				pLogger->LogInfo(log);

				std::string logStr;
				for( int i=0; i<amount; i++) {
					logStr.push_back(stage.buffer[stage.byteAmount + i]);
				}
				pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead content: " + logStr);
			}

			stage.byteAmount += amount;

			if(stage.byteAmount < PACKET_SIZE)
			{
				//partial packet
				stage.inputTimeStamp.update(); //update time stamp
			}
			else
			{
				//a complete packet is received.
				unsigned short crc;
				unsigned char crcLow, crcHigh;

				crc = stage.crc16.GetCRC(stage.buffer, PACKET_SIZE - 4);
				crcLow = crc & 0xff;
				crcHigh = (crc >> 8) & 0xff;

				if((crcLow == UcharFromHex(stage.buffer[PACKET_SIZE - 3], stage.buffer[PACKET_SIZE -4])) &&
					(crcHigh == UcharFromHex(stage.buffer[PACKET_SIZE -1], stage.buffer[PACKET_SIZE -2])))
				{
					//correct packet
					unsigned char tag = UcharFromHex(stage.buffer[1], stage.buffer[0]);
					unsigned char curPacketId = UcharFromHex(stage.buffer[3], stage.buffer[2]);

					if(tag == DATA_PACKET_TAG)
					{
						if(stage.previousId != curPacketId)
						{
							unsigned char dataLength = UcharFromHex(stage.buffer[5], stage.buffer[4]);

							dataLength = dataLength * 2;//convert to length of HEX bytes.
							for(unsigned char i=0; i<dataLength; i+=2) {
								appData.push_back(UcharFromHex(stage.buffer[6 + i + 1], stage.buffer[6 + i])); //read data in packet
							}
						}
						stage.previousId = curPacketId;
						{
							char log[256];
							sprintf(log, "CDeviceManager::onDeviceCanBeRead << %02x D %02x", stage.state, curPacketId);
							pLogger->LogInfo(log);

//							std::string packetStr = "CDeviceManager::onDeviceCanBeRead packet: ";
//							for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//								char buffer[8];
//								sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//								packetStr = packetStr + buffer;
//							}
//							pLogger->LogInfo(packetStr);
						}
						stage.state = INPUT_ACKNOWLEDGING;
					}
					else if(tag == ACK_PACKET_TAG)
					{
						device.dataExchange.outputStage.OnAcknowledgment(curPacketId);
						{
							char log[256];
							sprintf(log, "CDeviceManager::onDeviceCanBeRead << %02x A %02x", stage.state, curPacketId);
							pLogger->LogInfo(log);

//							std::string packetStr = "CDeviceManager::onDeviceCanBeRead packet: ";
//							for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//								char buffer[8];
//								sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//								packetStr = packetStr + buffer;
//							}
//							pLogger->LogInfo(packetStr);
						}
					}
					else {
						pLogger->LogError("CDeviceManager::onDeviceCanBeRead unknown packet: " + std::to_string(stage.buffer[0]));
					}
				}
				else
				{
					//corrupted packet
					pLogger->LogError("CDeviceManager::onDeviceCanBeRead << corrupted packet");
				}

				stage.byteAmount = 0;
			}
		}
	}

	if(appData.empty()) {
		return;
	}

	if(device.state < DeviceState::RECEIVING_NAME) {
		pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead clearing buffer, discard bytes: " + std::to_string(appData.size()));
		return;
	}

	//log data received
	for(unsigned int i=0; i<appData.size(); i++)
	{
		char tmpBuffer[16];

		sprintf(tmpBuffer, "%02x,", appData[i]);
		binaryLog = binaryLog + tmpBuffer;
		charLog.push_back(appData[i]);

		device.incoming.push_back(appData[i]);
	}
//	pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead binary APP content: " + binaryLog);
//	pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead char APP content: " + charLog);

	//notify upper layer of complete replies
	for(;;)
	{
		bool replyReady = false;

		//check if there is a complete reply
		for(auto it = device.incoming.begin(); it!=device.incoming.end(); it++) {
			if((*it == 0x0D) || (*it == 0x0A)) {
				//0x0D is changed to 0x0A in raspberry pi
				replyReady = true; //a reply is found.
				break;
			}
		}

		if(!replyReady) {
			break; //no complete reply, break the loop.
		}
		else
		{
			std::string reply;
			bool illegal = false;

			//retrieve the first reply from the incoming deque.
			for(; device.incoming.size() > 0; )
			{
				unsigned char c = device.incoming.front();
				device.incoming.pop_front(); //delete the first character.

				if((c >= ' ') && (c <= '~')) {
					reply.push_back(c);
				}
				else if((c == 0x0D) || (c == 0x0A)) {
					// a carriage return means that a complete reply is found
					//0x0D is changed to 0x0A in raspberry pi.
					if(illegal) {
						pLogger->LogError("CDeviceManager::onDeviceCanBeRead illegal reply from: " + device.fileName + " : " + reply.data());
					}
					else if(!reply.empty()) {
						pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead rely: " + device.fileName + ":" + std::string(reply.data()));
						onReply(device, reply);
					}
					break;
				}
				else {
					//illegal character
					if(illegal == false) {
						char tmpBuffer[512];
						sprintf(tmpBuffer, "CDeviceManager::onDeviceCanBeRead illegal character 0x%02x from: %s", c, device.fileName.c_str());
						pLogger->LogError(tmpBuffer);
						illegal = true;
					}
					reply.push_back(ILLEGAL_CHARACTER_REPLACEMENT);
				}
			}
		}
	}
}

void CDeviceManager::enqueueCommand(struct Device& device, const char * pCommand)
{
	if(pCommand == nullptr) {
		return;
	}

	std::string command(pCommand);
	enqueueCommand(device, command);
}

void CDeviceManager::enqueueCommand(struct Device& device, const std::string command)
{
	if(command.size() < 1) {
		pLogger->LogError("CDeviceManager::enqueueCommand empty command to device: " + device.fileName);
		return;
	}

	pLogger->LogDebug("CDeviceManager::enqueueCommand enqueue command: " + command + " : " + device.fileName);
	for(auto it=command.begin(); it!=command.end(); it++) {
		device.outgoing.push_back(*it);
	}
	device.outgoing.push_back(COMMAND_TERMINATER);
}

//write a command to device
void CDeviceManager::onDeviceCanBeWritten(struct Device& device)
{
	if(device.dataExchange.outputStage.state == OUTPUT_WAITING_ACK)
	{
		auto & stage = device.dataExchange.outputStage;

		if(stage.ackTimeStamp.elapsed() >= DATA_ACK_TIMEOUT)
		{
			//no acknowledgment in time, re-send this packet.
			for(unsigned int i=0; i<PACKET_SIZE; i++) {
				stage.buffer[i] = stage.packet[i];
			}
			stage.state = OUTPUT_SENDING;
			stage.sendingIndex = 0;

			{
				char buffer[256];

				sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten prepare re-send data packet: %02x", UcharFromHex(stage.buffer[3], stage.buffer[2]));
				pLogger->LogInfo(buffer);
			}
		}
	}

	if(device.dataExchange.inputStage.state == INPUT_ACKNOWLEDGING)
	{
		//trying to send out ACK
		if(device.dataExchange.outputStage.SendAcknowledgment(device.dataExchange.inputStage.previousId)) {
			device.dataExchange.inputStage.state = INPUT_RECEIVING;
		}
	}

	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex); //protect device.outgoing from incoming command

		switch(device.state)
		{
			case DeviceState::OPENED:
			{
				std::string command;

				//make device to spit out rubbish in receiving buffer.
				command.push_back(COMMAND_TERMINATER);
				enqueueCommand(device, command);
				pLogger->LogInfo("CDeviceManager::onDeviceCanBeWritten clearing device buffer: " + device.fileName);
				device.state = DeviceState::CLEARING_BUFFER;
				device.bufferCleaningStamp.update();
			}
			break;

			case DeviceState::CLEARING_BUFFER:
			{
				if(device.bufferCleaningStamp.elapsed() > 1000000) {
					//1 second is enough for device to spit out rubbish in receiving buffer.
					device.state = DeviceState::BUFFER_CLEARED;
					pLogger->LogInfo("CDeviceManager::onDeviceCanBeWritten cleared device buffer: " + device.fileName);
				}
			}
			break;

			case DeviceState::BUFFER_CLEARED:
			{
				//internal command to query device name
				std::string command(COMMAND_QUERY_NAME);
				enqueueCommand(device, command);
				device.state = DeviceState::RECEIVING_NAME;
				pLogger->LogInfo("CDeviceManager::onDeviceCanBeWritten querying device name: " + device.fileName);
			}
			break;

			default:
			{
				//nothing to do
			}
			break;
		}

		if(device.outgoing.size() > 0)
		{
			//something needs to be sent out
			device.dataExchange.outputStage.SendData(device.outgoing);
		}
	}

	// send data
	switch(device.dataExchange.outputStage.state)
	{
		case OUTPUT_SENDING:
		case OUTPUT_WAITING_ACK_WHILE_SENDING:
		{
			auto& stage = device.dataExchange.outputStage;

			unsigned char * pData = stage.buffer + stage.sendingIndex;
			unsigned int length = PACKET_SIZE - stage.sendingIndex;
			int amount = 0;

			for(unsigned int i=0; i<length; i++)
			{
				//send data byte by byte.
				int rc = write(device.fd, pData + i, 1);
				if(rc == 1) {
					amount++;
				}
				else {
					break;
				}
			}
			if(amount > 0)
			{
				stage.sendingIndex += amount;
				pLogger->LogInfo("CDeviceManager::onDeviceCanBeWritten wrote " + std::to_string(amount) + " bytes to " + device.fileName);
				{
					std::string logContent = "CDeviceManager::onDeviceCanBeWritten content: ";

					for(int i=0; i<amount; i++)
					{
						logContent.push_back(pData[i]);
			}
					pLogger->LogInfo(logContent);
				}
			}
			else {
				auto errorNumber = errno;
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten failed in writing " + device.fileName + ", errno: " + std::to_string(errorNumber));
			}
		}
		break;

		default:
			break;
	}

	//state
	if(device.dataExchange.outputStage.state == OUTPUT_SENDING)
	{
		auto& stage = device.dataExchange.outputStage;

		if(stage.sendingIndex < PACKET_SIZE)
		{
			//packet hasn't been sent completely, keep state unchanged.
		}
		else if(stage.sendingIndex == PACKET_SIZE)
		{
			unsigned char tag = UcharFromHex(stage.buffer[1], stage.buffer[0]);
			unsigned char packetId = UcharFromHex(stage.buffer[3], stage.buffer[2]);

			if(tag == DATA_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x D %02x", stage.state, packetId);
					pLogger->LogInfo(buffer);

//					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
//					for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//						char buffer[8];
//						sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//						packetStr = packetStr + buffer;
//					}
//					pLogger->LogInfo(packetStr);
				}
				stage.ackTimeStamp.update();
				stage.state = OUTPUT_WAITING_ACK; // a data packet was sent, wait for the ACK.
			}
			else if(tag == ACK_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x A %02x", stage.state, packetId);
					pLogger->LogInfo(buffer);

//					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
//					for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//						char buffer[8];
//						sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//						packetStr = packetStr + buffer;
//					}
//					pLogger->LogInfo(packetStr);
				}
				stage.state = OUTPUT_IDLE; // an ACK was sent out.
			}
			else {
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten packet of unknown type was sent: " + std::to_string(tag));
				stage.state = OUTPUT_IDLE;
			}
		}
		else if(stage.sendingIndex > PACKET_SIZE)
		{
			pLogger->LogError("CDeviceManager::onDeviceCanBeWritten extra data was sent: " + std::to_string(stage.sendingIndex));
			stage.state = OUTPUT_IDLE;
		}
	}
	else if(device.dataExchange.outputStage.state == OUTPUT_WAITING_ACK_WHILE_SENDING)
	{
		auto& stage = device.dataExchange.outputStage;

		if(stage.sendingIndex < PACKET_SIZE)
		{
			//packet hasn't been sent completely, keep state unchanged.
		}
		else if(stage.sendingIndex == PACKET_SIZE)
		{
			unsigned char tag = UcharFromHex(stage.buffer[1], stage.buffer[0]);
			unsigned char packetId = UcharFromHex(stage.buffer[3], stage.buffer[2]);

			if(tag == DATA_PACKET_TAG)
			{
				//no data packet can be sent while waiting for acknowledgment
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x D %02x", stage.state, packetId);
					pLogger->LogInfo(buffer);

//					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
//					for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//						char buffer[8];
//						sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//						packetStr = packetStr + buffer;
//					}
//					pLogger->LogInfo(packetStr);
				}
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten wrong data packet was sent to: " + device.deviceName);
			}
			else if(tag == ACK_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x A %02x", stage.state, packetId);
					pLogger->LogInfo(buffer);

//					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
//					for(unsigned int i=0; i<PACKET_SIZE; i+=2) {
//						char buffer[8];
//						sprintf(buffer, "%c%c,", stage.buffer[i], stage.buffer[i+1]);
//						packetStr = packetStr + buffer;
//					}
//					pLogger->LogInfo(packetStr);
				}
			}
			else
			{
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten packet of unknown type was sent: " + std::to_string(tag));
			}

			stage.state = OUTPUT_WAITING_ACK; //continue waiting for acknowledgment.
		}
		else if(stage.sendingIndex > PACKET_SIZE)
		{
			pLogger->LogError("CDeviceManager::onDeviceCanBeWritten extra data was sent: " + std::to_string(stage.sendingIndex));
			stage.state = OUTPUT_IDLE;
		}
	}
}

void CDeviceManager::onDeviceError(struct Device& device, int errorNumber)
{
	pLogger->LogError("CDeviceManager device error: " + device.fileName + ", errno: " + std::to_string(errorNumber));
	device.state = DeviceState::ERROR;
}

void CDeviceManager::pollDevices()
{
	if(_devices.size() < 1) {
		return;
	}

	std::vector<struct pollfd> fdVector;

	//poll reading
	for(size_t i=0; i<_devices.size(); i++)
	{
		pollfd fd;

		fd.fd = _devices[i].fd;
		fd.events = POLLIN | POLLERR;
		fd.revents = 0;
		fdVector.push_back(fd);
	}
	auto rc = poll(fdVector.data(), fdVector.size(), 10); //sleep 10 milliseconds if no event
	auto errorNumber = errno;
	if(rc == 0) {
		; //no event happened, jump to writing polling
	}
	else if(rc < 0) {
		sleep(100); //
		return;
	}
	else // rc > 0
	{
		for(size_t i=0; i<_devices.size(); i++)
		{
			auto events = fdVector[i].revents;

			if(events & POLLERR) {
				onDeviceError(_devices[i], errorNumber);
			}
			else
			{
				if(events & POLLIN) {
					//device can be read.
					_devices[i].readStamp.update();
					onDeviceCanBeRead(_devices[i]);
				}
			}

			if(_devices[i].readStamp.elapsed() > _devices[i].FileReadWarningThreshold)
			{
				_devices[i].readStamp.update();
				//pLogger->LogError("CDeviceManager::pollDevices reading unavailable: " + _devices[i].fileName);
			}
		}
	}
	//remove the device having an error
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);
		for(auto deviceIt = _devices.begin(); deviceIt != _devices.end(); )
		{
			if(deviceIt->state == DeviceState::ERROR)
			{
				close(deviceIt->fd);
				//notify observers of device's unavailability
				if(!deviceIt->deviceName.empty()) {
					//this device has been fully enumerated.
					_pObserver->OnDeviceUnplugged(deviceIt->deviceName);
				}
				//erase this device
				deviceIt = _devices.erase(deviceIt);
			}
			else
			{
				deviceIt++;
			}
		}
	}

	fdVector.clear();
	//poll writing
	for(size_t i=0; i<_devices.size(); i++)
	{
		pollfd fd;

		fd.fd = _devices[i].fd;
		fd.events = POLLOUT | POLLERR;
		fd.revents = 0;
		fdVector.push_back(fd);
	}
	rc = poll(fdVector.data(), fdVector.size(), 0); //return at once.
	errorNumber = errno;
	if(rc == 0)
	{
		//check period of writing unavailable
		for(size_t i=0; i<_devices.size(); i++)
		{
			if(_devices[i].writeStamp.elapsed() > _devices[i].FileWriteWarningThreshold)
			{
				_devices[i].writeStamp.update();
				pLogger->LogError("CDeviceManager::pollDevices writing unavailable: " + _devices[i].fileName);
			}
		}
	}
	else if(rc < 0) {
		sleep(100); //
		return;
	}
	else // rc > 0
	{
		for(size_t i=0; i<_devices.size(); i++)
		{
			auto events = fdVector[i].revents;

			if(events & POLLERR) {
				onDeviceError(_devices[i], errorNumber);
			}
			else
			{
				if(events & POLLOUT) {
					//device can be written.
					_devices[i].writeStamp.update();
					onDeviceCanBeWritten(_devices[i]);
				}
			}

			//check period of writing unavailable
			if(_devices[i].writeStamp.elapsed() > _devices[i].FileWriteWarningThreshold)
			{
				_devices[i].writeStamp.update();
				pLogger->LogError("CDeviceManager::pollDevices writing unavailable: " + _devices[i].fileName);
			}
		}
	}
	//remove the device having an error
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);
		for(auto deviceIt = _devices.begin(); deviceIt != _devices.end(); )
		{
			if(deviceIt->state == DeviceState::ERROR)
			{
				close(deviceIt->fd);
				//notify observers of device's unavailability
				if(!deviceIt->deviceName.empty()) {
					//this device has been fully enumerated.
					_pObserver->OnDeviceUnplugged(deviceIt->deviceName);
				}
				//erase this device
				deviceIt = _devices.erase(deviceIt);
			}
			else
			{
				deviceIt++;
			}
		}
	}
}

void CDeviceManager::runTask()
{
	pLogger->LogInfo("CDeviceManager::runTask starts");

	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			if(!_startMonitoringDevices) {
				sleep(10);
				continue;
			}

			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);
				if(_pObserver == NULL) {
					sleep(10);
					continue;
				}
			}

			checkDevices();
			pollDevices();

			if(_devices.empty()) {
				sleep(100);
			}
		}
	}

	pLogger->LogInfo("CDeviceManager::runTask exited");
}

CDeviceManager::DataInputStage::DataInputStage()
{
	state = INPUT_RECEIVING;
	byteAmount = 0;
	previousId = INVALID_PACKET_ID;
}

CDeviceManager::DataOutputStage::DataOutputStage()
{
	state = OUTPUT_IDLE;
	sendingIndex = 0;
	packetId = INITAL_PACKET_ID;
}

void CDeviceManager::DataOutputStage::IncreasePacketId()
{
	packetId++;

	if(packetId == INVALID_PACKET_ID) {
		packetId++; //jump over the invalid packet id
	}
	if(packetId == INITAL_PACKET_ID) {
		packetId++; //jump over the intial packet id
	}
}

void CDeviceManager::DataOutputStage::OnAcknowledgment(unsigned char packetId)
{
	unsigned char pId = UcharFromHex(packet[3], packet[2]);

	if(state == OUTPUT_WAITING_ACK)
	{
		if(pId == packetId) {
			state = OUTPUT_IDLE;
			//pLogger->LogInfo("CDeviceManager::DataOutputStage::OnNotify " + std::to_string(packetId));
		}
	}
	else if(state == OUTPUT_WAITING_ACK_WHILE_SENDING)
	{
		if(pId == packetId) {
			state = OUTPUT_SENDING;
			//pLogger->LogInfo("CDeviceManager::DataOutputStage::OnNotify " + std::to_string(packetId));
		}
	}
}

bool CDeviceManager::DataOutputStage::SendAcknowledgment(unsigned char packetId)
{
	uint16_t crc;
	unsigned char l4, h4;

	if((state != OUTPUT_IDLE) && (state != OUTPUT_WAITING_ACK)) {
		return false; //cannot send ack
	}

	//fill buffer with an ACK
	//Tag
	UcharToHex(ACK_PACKET_TAG, l4, h4);
	buffer[0] = h4;
	buffer[1] = l4;
	//packetId
	UcharToHex(packetId, l4, h4);
	buffer[2] = h4;
	buffer[3] = l4;
	//paddings
	UcharToHex(0, l4, h4);
	for(unsigned int i=4; i<(PACKET_SIZE -4); i+=2) {
		buffer[i] = h4;
		buffer[i + 1] = l4;
	}
	//calculate CRC
	crc = crc16.GetCRC(buffer, PACKET_SIZE -4);
	UcharToHex(crc & 0xff, l4, h4);
	buffer[PACKET_SIZE -4] = h4;
	buffer[PACKET_SIZE -3] = l4;
	UcharToHex((crc >> 8) & 0xff, l4, h4);
	buffer[PACKET_SIZE -2] = h4;
	buffer[PACKET_SIZE -1] = l4;

	{
		char log[256];

		sprintf(log, "CDeviceManager::DataOutputStage::SendAcknowledgment prepare ACK packet: %02x", UcharFromHex(buffer[3], buffer[2]));
		pLogger->LogInfo(log);
	}

	if(state == OUTPUT_IDLE) {
		state = OUTPUT_SENDING;
	}
	else if(state == OUTPUT_WAITING_ACK) {
		state = OUTPUT_WAITING_ACK_WHILE_SENDING;
	}
	sendingIndex = 0;

	return true;
}

void CDeviceManager::DataOutputStage::SendData(std::deque<char>& dataQueue)
{
	if(state != OUTPUT_IDLE) {
		return;
	}
	if(dataQueue.empty()) {
		return;
	}

	unsigned char amount;
	unsigned char c;
	uint16_t crc;
	unsigned char l4, h4;

	//fill packet with application data
	//Tag
	UcharToHex(DATA_PACKET_TAG, l4, h4);
	packet[0] = h4;
	packet[1] = l4;
	//packetId
	UcharToHex(packetId, l4, h4);
	packet[2] = h4;
	packet[3] = l4;
	//data
	for(amount=0; amount<(PACKET_SIZE-10); amount+=2)
	{
		if(dataQueue.empty()) {
			break;
		}

		c = dataQueue[0];
		dataQueue.pop_front();
		UcharToHex(c, l4, h4);
		packet[6 + amount] = h4;
		packet[6 + amount + 1] = l4;
	}
	//length
	UcharToHex(amount/2, l4, h4);
	packet[4] = h4;
	packet[5] = l4;
	//padding
	for(; amount<(PACKET_SIZE-10); amount+=2) {
		UcharToHex(0, l4, h4);
		packet[6 + amount] = h4;
		packet[6 + amount + 1] = l4;
	}
	//crc
	crc = crc16.GetCRC(packet, PACKET_SIZE - 4);
	UcharToHex(crc & 0xff, l4, h4); //crcLow
	packet[PACKET_SIZE -4] = h4;
	packet[PACKET_SIZE -3] = l4;
	UcharToHex((crc >> 8) & 0xff, l4, h4); //crcHigh
	packet[PACKET_SIZE -2] = h4;
	packet[PACKET_SIZE -1] = l4;

	//copy packet to buffer
	for(unsigned int i=0; i<PACKET_SIZE; i++) {
		buffer[i] = packet[i];
	}

	{
		char log[256];

		sprintf(log, "CDeviceManager::DataOutputStage::SendData prepare data packet: %02x", UcharFromHex(buffer[3], buffer[2]));
		pLogger->LogInfo(log);
	}

	state = OUTPUT_SENDING;
	sendingIndex = 0;

	IncreasePacketId();
}


