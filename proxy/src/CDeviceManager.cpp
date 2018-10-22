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
#include "ProxyLogger.h"
#include <poll.h>
#include <stddef.h>
#include <unistd.h>

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
			pLogger->LogError(std::string(__FUNCTION__) + " illegal character in command: " + deviceName + ":" + command);
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
			}
		}
	}
	if(it == _devices.end()) {
		pLogger->LogError("CDeviceManager::SendCommand failed in sending: " + deviceName + ":" + command);
	}
}

void CDeviceManager::checkDevices()
{
	std::vector<std::string> deviceUnpluged;
	std::vector<std::string> currentFileList;

	//iterate each file in the folder /dev/serial/device_by_id
	try
	{
		Poco::File folderFile(DEVICE_FOLDER_PATH);

		if(folderFile.exists() && folderFile.isDirectory())
		{
			//open all DCD files.
			DirectoryIterator end;
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
		pLogger->Log(std::string(__FUNCTION__) + "exception: " + exc.displayText());
	}
	catch(...)
	{
		pLogger->LogError(std::string(__FUNCTION__) + " unknown exception");
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
	switch(device.state)
	{
		case DeviceState::RECEIVING_NAME:
		{
			device.deviceName = reply;
			device.state = DeviceState::ACTIVE;
			pLogger->LogInfo("CDeviceManager::onReply device insertion: " + device.deviceName);
			_pObserver->OnDeviceInserted(device.deviceName);
		}
		break;

		case DeviceState::ACTIVE:
		{
			_pObserver->OnDeviceReply(device.deviceName, reply);
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
void CDeviceManager::onDeviceInput(struct Device& device)
{
	const int BUFFER_SIZE = 1024;
	unsigned char buffer[BUFFER_SIZE];
	ssize_t amount;

	//read data from device
	for(;;)
	{
		amount =  read(device.fd, buffer, BUFFER_SIZE);
		pLogger->LogDebug("CDeviceManager::onDeviceInput " + std::to_string(amount) + " from " + device.fileName);

		for(int i=0; i<amount; i++)
		{
			char tmpBuffer[256];

			sprintf(tmpBuffer, "CDeviceManager::onDeviceInput 0x%02x:%c", buffer[i], buffer[i]);
			pLogger->LogDebug(tmpBuffer);
			device.incoming.push_back(buffer[i]);
		}
		if(amount < 1) {
			break;
		}
	}

	//notify mapping of complete replies
	for(;;)
	{
		bool replyReady = false;

		//check if there is a complete reply
		for(auto it = device.incoming.begin(); it!=device.incoming.end(); it++) {
			if(*it == '\n') {
				replyReady = true;
				break;
			}
		}

		if(replyReady)
		{
			std::vector<char> reply;
			bool illegal = false;

			//retrieve the reply from the incoming deque.
			for(; device.incoming.size() > 0; )
			{
				unsigned char c = device.incoming.front();
				device.incoming.pop_front();

				if((c >= ' ') && (c <= '~')) {
					reply.push_back(c);
				}
				else if(c == '\r') {
					//ignore this character
					continue;
				}
				else if(c == '\n') {
					//a complete reply is found
					if(illegal) {
						pLogger->LogError("CDeviceManager illegal reply: " + device.fileName + ":" + reply.data());
					}
					else {
						onReply(device, std::string(reply.data()));
						pLogger->LogDebug("CDeviceManager rely: " + device.fileName + ":" + std::string(reply.data()));
					}
					break;
				}
				else {
					//illegal character
					if(illegal == false) {
						pLogger->LogError("CDeviceManager illegal character in : " + device.fileName);
						illegal = true;
					}
					reply.push_back(ILLEGAL_CHARACTER_REPLACEMENT);
				}
			}
		}
		else {
			break;
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
void CDeviceManager::onDeviceOutput(struct Device& device)
{
	int amount;
	char c;

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	//internal command to query device name
	{
		switch(device.state)
		{
			case DeviceState::OPENED:
			{
				std::string command(COMMAND_QUERY_NAME);
				enqueueCommand(device, command);
				device.state = DeviceState::RECEIVING_NAME;
			}
			break;

			default:
			{
				//nothing to do
			}
			break;
		}
	}

	if(device.outgoing.size() > 0)
	{
		char buffer[512];
		sprintf(buffer, "CDeviceManager::onDeviceOutput write %ld bytes to device file: %s", device.outgoing.size(), device.fileName.c_str());
		pLogger->LogDebug(buffer);

		//send command to device
		for(;;)
		{
			if(device.outgoing.empty()) {
				break;
			}

			//write a byte to device
			c = device.outgoing.front();
			amount = write(device.fd, &c, 1);
			if(amount > 0) {
				//byte is written
				sprintf(buffer, "CDeviceManager::onDeviceOutput wrote: 0x%02x : %c", c ,c);
				pLogger->LogDebug(buffer);
				device.outgoing.pop_front();
			}
			else {
				//nothing is written
				break;
			}
		}
	}
}

void CDeviceManager::onDeviceError(struct Device& device)
{
	pLogger->LogError("CDeviceManager device error: " + device.fileName);
}



void CDeviceManager::pollDevices()
{
	if(_devices.size() < 1) {
		return;
	}
	else
	{
		std::vector<struct pollfd> fdVector;

		//poll devices output.
		for(size_t i=0; i<_devices.size(); i++)
		{
			pollfd fd;

			fd.fd = _devices[i].fd;
			fd.events = POLLOUT | POLLERR;
			fd.revents = 0;
			fdVector.push_back(fd);
		}
		auto rc = poll(fdVector.data(), fdVector.size(), 10);
		if(rc > 0)
		{
			for(size_t i=0; i<_devices.size(); i++)
			{
				auto events = fdVector[i].revents;

				if(events & POLLOUT) {
					onDeviceOutput(_devices[i]);
				}
				if(events & POLLERR) {
					onDeviceError(_devices[i]);
				}
			}
		}

		//poll devices input
		fdVector.clear();
		for(size_t i=0; i<_devices.size(); i++)
		{
			pollfd fd;

			fd.fd = _devices[i].fd;
			fd.events = POLLIN | POLLERR;
			fd.revents = 0;
			fdVector.push_back(fd);
		}

		rc = poll(fdVector.data(), fdVector.size(), 10);
		if(rc > 0)
		{
			for(size_t i=0; i<_devices.size(); i++)
			{
				auto events = fdVector[i].revents;

				if(events & POLLIN) {
					onDeviceInput(_devices[i]);
				}
				if(events & POLLERR) {
					onDeviceError(_devices[i]);
				}
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
