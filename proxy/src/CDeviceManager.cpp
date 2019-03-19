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
	std::vector<std::string> currentFileList;

	//iterate each file in the folder /dev/serial/device_by_id
	try
	{
		Poco::File folderFile(DEVICE_FOLDER_PATH);

		if(folderFile.exists() && folderFile.isDirectory())
		{
			//open all DCD files.
			Poco::DirectoryIterator end;
			for(DirectoryIterator it(DEVICE_FOLDER_PATH) ;it != end; it++)
			{
				bool fileIsOpened = false;
				Path p(it->path());
				int fd;

				//check if current device has been opened
				for(size_t i = 0; i<_devices.size(); i++) {
					if(_devices[i].fileName == p.getFileName()) {
						fileIsOpened = true;
						break;
					}
				}
				if(fileIsOpened) {
					currentFileList.push_back(p.getFileName());
					continue;
				}

				//check if it is a DCD device
				std::string fileName = it->path();
				if(fileName.find(IDENTIFIER) == std::string::npos) {
					// not a DCD device
					continue;
				}

				//open device
				fd = open(fileName.c_str(), O_RDWR | O_NOCTTY);
				if(fd < 0){
					//cannot open device file
					continue;
				}

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
					rc = cfsetspeed(&tios, B1152000); //intentionally use a larger baudrate, hoping that more buffer can be allocated
					if(0 != rc)
					{
						auto e = errno;
						pLogger->LogError("CDeviceManager::checkDevices cfsetspeed errno: " + std::to_string(e));
						close(fd);
						continue;
					}
					//c_iflag
					tios.c_iflag &= ~ICRNL;
					tios.c_iflag &= ~IXON;
					tios.c_iflag |= IGNPAR;
					//c_oflag
					tios.c_oflag &= ~ONLCR;
					tios.c_oflag &= ~OPOST;
					//c_cflag
					tios.c_cflag &= ~CLOCAL;
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

				currentFileList.push_back(p.getFileName());
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
			auto newDeviceIt = currentFileList.begin();
			for(; newDeviceIt!=currentFileList.end(); newDeviceIt++)
			{
				if(oldDeviceIt->fileName == *newDeviceIt) {
					break;
				}
			}
			if(newDeviceIt == currentFileList.end()) {
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
		ssize_t amount;
		auto & stage = device.dataExchange.inputStage;

		if(stage.byteAmount > 0)
		{
			//a partial packet is in buffer
			if(stage.timeStamp.elapsed() >= DATA_INPUT_TIMEOUT)
			{
				pLogger->LogError("CDeviceManager::onDeviceCanBeRead packet receiving timeout");
				stage.byteAmount = 0;
			}
		}

		amount = read(device.fd, stage.buffer + stage.byteAmount, PACKET_SIZE - stage.byteAmount);
		errorNumber = errno;
		if(amount == 0) {
			pLogger->LogError("CDeviceManager::onDeviceCanBeRead nothing is read, errno: " + std::to_string(errorNumber));
		}
		else if(amount < 0) {
			pLogger->LogError("CDeviceManager::onDeviceCanBeRead error in device, errno: " + std::to_string(errorNumber));
		}
		else
		{
			{
				char log[256];
				sprintf(log, "CDeviceManager::onDeviceCanBeRead %d bytes from %s", amount, device.fileName.c_str());
				pLogger->LogInfo(log);
			}

			stage.byteAmount += amount;

			if(stage.byteAmount < PACKET_SIZE)
			{
				//partial packet
				stage.timeStamp.update(); //update time stamp
			}
			else
			{
				//a complete packet is received.
				unsigned short crc;
				unsigned char crcLow, crcHigh;

				crc = stage.crc16.GetCRC(stage.buffer, PACKET_SIZE -2);
				crcLow = crc & 0xff;
				crcHigh = (crc >> 8) & 0xff;

				if((crcLow == stage.buffer[PACKET_SIZE -2]) && (crcHigh == stage.buffer[PACKET_SIZE -1]))
				{
					//correct packet
					if(stage.buffer[0] == DATA_PACKET_TAG)
					{
						if(stage.previousId != stage.buffer[1])
						{
							for(unsigned char i=0; i<stage.buffer[2]; i++) {
								appData.push_back(stage.buffer[3 + i]); //read data in packet
							}
						}
						stage.previousId = stage.buffer[1];
						{
							char log[256];
							sprintf(log, "CDeviceManager::onDeviceCanBeRead << %02x D %02x", stage.state, stage.previousId);
							pLogger->LogInfo(log);

							std::string packetStr = "CDeviceManager::onDeviceCanBeRead packet: ";
							for(unsigned int i=0; i<PACKET_SIZE; i++) {
								char buffer[8];
								sprintf(buffer, "%02x,", stage.buffer[i]);
								packetStr = packetStr + buffer;
							}
							pLogger->LogInfo(packetStr);
						}
						stage.state = INPUT_ACKNOWLEDGING;
					}
					else if(stage.buffer[0] == ACK_PACKET_TAG)
					{
						device.dataExchange.outputStage.OnAcknowledgment(stage.buffer[1]);
						{
							char log[256];
							sprintf(log, "CDeviceManager::onDeviceCanBeRead << %02x A %02x", stage.state, stage.buffer[1]);
							pLogger->LogInfo(log);

							std::string packetStr = "CDeviceManager::onDeviceCanBeRead packet: ";
							for(unsigned int i=0; i<PACKET_SIZE; i++) {
								char buffer[8];
								sprintf(buffer, "%02x,", stage.buffer[i]);
								packetStr = packetStr + buffer;
							}
							pLogger->LogInfo(packetStr);
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
	pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead binary APP content: " + binaryLog);
	pLogger->LogInfo("CDeviceManager::onDeviceCanBeRead char APP content:" + charLog);

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

		if(stage.timeStamp.elapsed() >= DATA_ACK_TIMEOUT)
		{
			//no acknowledgment in time, re-send this packet.
			for(unsigned int i=0; i<PACKET_SIZE; i++) {
				stage.buffer[i] = stage.packet[i];
			}
			stage.state = OUTPUT_SENDING;
			stage.sendingIndex = 0;

			{
				char buffer[256];

				sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten prepare re-send data packet: %02x", stage.buffer[1]);
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
				device.timeStamp.update();
			}
			break;

			case DeviceState::CLEARING_BUFFER:
			{
				if(device.timeStamp.elapsed() > 1000000) {
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

			auto amount = write(device.fd, pData, length);
			if(amount > 0) {
				stage.sendingIndex += amount;
				pLogger->LogInfo("CDeviceManager::onDeviceCanBeWritten wrote " + std::to_string(amount) + " bytes to " + device.fileName);
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
			if(stage.buffer[0] == DATA_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x D %02x", stage.state, stage.buffer[1]);
					pLogger->LogInfo(buffer);

					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
					for(unsigned int i=0; i<PACKET_SIZE; i++) {
						char buffer[8];
						sprintf(buffer, "%02x,", stage.buffer[i]);
						packetStr = packetStr + buffer;
					}
					pLogger->LogInfo(packetStr);
				}
				stage.timeStamp.update();
				stage.state = OUTPUT_WAITING_ACK; // a data packet was sent, wait for the ACK.
			}
			else if(stage.buffer[0] == ACK_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x A %02x", stage.state, stage.buffer[1]);
					pLogger->LogInfo(buffer);

					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
					for(unsigned int i=0; i<PACKET_SIZE; i++) {
						char buffer[8];
						sprintf(buffer, "%02x,", stage.buffer[i]);
						packetStr = packetStr + buffer;
					}
					pLogger->LogInfo(packetStr);
				}
				stage.state = OUTPUT_IDLE; // an ACK was sent out.
			}
			else {
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten packet of unknown type was sent");
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
			if(stage.buffer[0] == DATA_PACKET_TAG)
			{
				//no data packet can be sent while waiting for acknowledgment
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x D %02x", stage.state, stage.buffer[1]);
					pLogger->LogInfo(buffer);

					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
					for(unsigned int i=0; i<PACKET_SIZE; i++) {
						char buffer[8];
						sprintf(buffer, "%02x,", stage.buffer[i]);
						packetStr = packetStr + buffer;
					}
					pLogger->LogInfo(packetStr);
				}
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten wrong data packet was sent to: " + device.deviceName);
				stage.state = OUTPUT_IDLE;
			}
			else if(stage.buffer[0] == ACK_PACKET_TAG)
			{
				{
					char buffer[256];
					sprintf(buffer, "CDeviceManager::onDeviceCanBeWritten >> %02x A %02x", stage.state, stage.buffer[1]);
					pLogger->LogInfo(buffer);

					std::string packetStr = "CDeviceManager::onDeviceCanBeWritten packet: ";
					for(unsigned int i=0; i<PACKET_SIZE; i++) {
						char buffer[8];
						sprintf(buffer, "%02x,", stage.buffer[i]);
						packetStr = packetStr + buffer;
					}
					pLogger->LogInfo(packetStr);
				}
				stage.state = OUTPUT_WAITING_ACK; //continue waiting for acknowledgment.
			}
			else
			{
				pLogger->LogError("CDeviceManager::onDeviceCanBeWritten packet of unknown type was sent");
				stage.state = OUTPUT_IDLE;
			}
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

	//check if device can be written.
	for(size_t i=0; i<_devices.size(); i++)
	{
		pollfd fd;

		fd.fd = _devices[i].fd;
		fd.events = POLLOUT | POLLERR;
		fd.revents = 0;
		fdVector.push_back(fd);
	}
	auto rc = poll(fdVector.data(), fdVector.size(), 0);
	auto errorNumber = errno;
	if(rc > 0)
	{
		for(size_t i=0; i<_devices.size(); i++)
		{
			auto events = fdVector[i].revents;

			if(events & POLLOUT) {
				//device can be written.
				onDeviceCanBeWritten(_devices[i]);
			}
			if(events & POLLERR) {
				onDeviceError(_devices[i], errorNumber);
			}
		}
	}

	//check if device can be read
	fdVector.clear();
	for(size_t i=0; i<_devices.size(); i++)
	{
		pollfd fd;

		fd.fd = _devices[i].fd;
		fd.events = POLLIN | POLLERR;
		fd.revents = 0;
		fdVector.push_back(fd);
	}

	rc = poll(fdVector.data(), fdVector.size(), 5);
	errorNumber = errno;
	if(rc > 0)
	{
		for(size_t i=0; i<_devices.size(); i++)
		{
			auto events = fdVector[i].revents;

			if(events & POLLIN) {
				//device can be read.
				onDeviceCanBeRead(_devices[i]);
			}
			if(events & POLLERR) {
				onDeviceError(_devices[i], errorNumber);
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
	if(state == OUTPUT_WAITING_ACK)
	{
		if(packet[1] == packetId) {
			state = OUTPUT_IDLE;
			pLogger->LogInfo("CDeviceManager::DataOutputStage::OnNotify " + std::to_string(packetId));
		}
	}
	else if(state == OUTPUT_WAITING_ACK_WHILE_SENDING)
	{
		if(packet[1] == packetId) {
			state = OUTPUT_SENDING;
			pLogger->LogInfo("CDeviceManager::DataOutputStage::OnNotify " + std::to_string(packetId));
		}
	}
}

bool CDeviceManager::DataOutputStage::SendAcknowledgment(unsigned char packetId)
{
	uint16_t crc;

	if((state != OUTPUT_IDLE) && (state != OUTPUT_WAITING_ACK)) {
		return false; //cannot send ack
	}

	//fill buffer with an ACK
	buffer[0] = ACK_PACKET_TAG;
	buffer[1] = packetId;
	for(unsigned int i=2; i<(PACKET_SIZE -2); i++) {
		buffer[i] = 0;
	}

	//calculate CRC
	crc = crc16.GetCRC(buffer, PACKET_SIZE -2);

	buffer[PACKET_SIZE -2] = crc & 0xff;
	buffer[PACKET_SIZE -1] = (crc >> 8) & 0xff;

	{
		char buffer[256];

		sprintf(buffer, "CDeviceManager::DataOutputStage::SendAcknowledgment prepare ACK packet: %02x", buffer[1]);
		pLogger->LogInfo(buffer);
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

	//header
	packet[0] = DATA_PACKET_TAG;
	packet[1] = packetId;
	//data
	for(amount=0; amount<(PACKET_SIZE -5); amount++)
	{
		if(dataQueue.empty()) {
			break;
		}

		c = dataQueue[0];
		dataQueue.pop_front();
		packet[3 + amount] = c;
	}
	//length
	packet[2] = amount;
	//padding
	for(; amount<(PACKET_SIZE -5); amount++) {
		packet[3 + amount] = 0;
	}
	//crc
	crc = crc16.GetCRC(packet, PACKET_SIZE -2);
	packet[PACKET_SIZE -2] = crc & 0xff;
	packet[PACKET_SIZE -1] = (crc >> 8) & 0xff;

	for(unsigned int i=0; i<PACKET_SIZE; i++) {
		buffer[i] = packet[i];
	}

	{
		char buffer[256];

		sprintf(buffer, "CDeviceManager::DataOutputStage::SendData prepare data packet: %02x", packet[1]);
		pLogger->LogInfo(buffer);
	}

	state = OUTPUT_SENDING;
	sendingIndex = 0;

	IncreasePacketId();
}


