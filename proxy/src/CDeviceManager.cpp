/*
 * CDeviceManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */
#include "stddef.h"
#include "fcntl.h"
#include "CDeviceManager.h"
#include "CDeviceSocketMapping.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
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
	_pMapping = NULL;
	_startMonitoringDevices = false;
}

CDeviceManager::~CDeviceManager() {
	// TODO Auto-generated destructor stub
}

void CDeviceManager::SetDeviceSocketMapping(CDeviceSocketMapping * pMappingObj)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_pMapping = pMappingObj;
}

void CDeviceManager::StartMonitoringDevices()
{
	_startMonitoringDevices = true;
}

void CDeviceManager::SendCommand(const std::string& deviceName, const std::string& command)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	for(int i=0; i<_devices.size(); i++) {
		if(_devices[i].state == DeviceState::ACTIVE) {
			if(_devices[i].deviceName == deviceName) {
				pLogger->LogDebug("CDeviceManager enqueue command: " + deviceName + ":" + command);
				enqueueCommand(_devices[i], command);
			}
		}
	}
}

void CDeviceManager::checkDevices()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	//iterate each file in the folder /dev/serial/device_by_id
	try
	{
		DirectoryIterator end;
		std::vector<bool> devicesExist;

		//firstly, take it for granted that add _devices[i] is lost
		for(size_t i=0; i<_devices.size(); i++) {
			devicesExist.push_back(false);
		}

		//open all DCD files.
		for(DirectoryIterator it(DEVICE_FOLDER_PATH) ;it != end; it++)
		{
			bool fileIsOpened = false;
			Path p(it->path());
			int fd;

			//check if current device has been opened
			for(size_t i = 0; i<_devices.size(); i++) {
				if(_devices[i].fileName == p.getFileName()) {
					fileIsOpened = true;
					devicesExist[i] = true;
					break;
				}
			}
			if(fileIsOpened) {
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
			struct Device device;
			device.fd = fd;
			device.fileName = p.getFileName();
			device.state = DeviceState::OPENED;
			_devices.push_back(device);
			devicesExist.push_back(true);
		}

		//notify unplugged device files
		for(auto i = (devicesExist.size() -1); i >= 0; i--)
		{
			//if an active device is missing, then it is unplugged.
			if(devicesExist[i] == false) {
				if(_devices[i].state == DeviceState::ACTIVE) {
					_pMapping->OnDeviceUnplugged(_devices[i].deviceName); //notify mapping of the unplug
					devicesExist.erase(devicesExist.begin() + i); //remove the tag
					_devices.erase(_devices.begin() + i); // remove device from list.
				}
			}
		}

	}
	catch (Poco::Exception& exc)
	{
		pLogger->Log(exc.displayText());
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
			_pMapping->OnDeviceInserted(device.deviceName);
		}
		break;

		case DeviceState::ACTIVE:
		{
			_pMapping->OnDeviceReply(device.deviceName, reply);
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
		for(int i=0; i<amount; i++)
		{
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
		for(int i=0; i<device.incoming.size(); i++) {
			if(device.incoming[i] == '\n') {
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
					reply.push_back(ILLEGAL_CHARACTER_REPRESENTIVE);
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
		return;
	}

	for(int i=0; i<command.size(); i++) {
		if(command[i] == '\r') {
			continue; //skip '\r'
		}
		device.outgoing.push_back(command[i]);
	}
	device.outgoing.push_back(COMMAND_TERMINATER);
}


//write a command to device
void CDeviceManager::onDeviceOutput(struct Device& device)
{
	int amount;
	char c;

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
		//send command to device
		for(;;)
		{
			//write a byte to device
			c = device.outgoing.front();
			amount = write(device.fd, &c, 1);
			if(amount > 0) {
				//byte is written
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
		//poll devices.
		std::vector<struct pollfd> fdVector;

		for(size_t i=0; i<_devices.size(); i++)
		{
			pollfd fd;

			fd.fd = _devices[i].fd;
			fd.events = POLLIN | POLLOUT | POLLERR;
			fd.revents = 0;
			fdVector.push_back(fd);
		}

		auto rc = poll(fdVector.data(), fdVector.size(), 10);
		if(rc > 0)
		{
			for(size_t i=0; i<_devices.size(); i++)
			{
				auto events = fdVector[i].revents;

				if(events & POLLIN) {
					onDeviceInput(_devices[i]);
				}
				if(events & POLLOUT) {
					onDeviceOutput(_devices[i]);
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
				if(_pMapping == NULL) {
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
}
