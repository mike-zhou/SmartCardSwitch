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

using Poco::DirectoryIterator;
using Poco::File;
using Poco::Path;

extern ProxyLogger * pLogger;

CDeviceManager::CDeviceManager() : Task("CDeviceManager")
{
	DEVICE_FOLDER_PATH = "/dev/serial/by-id";
	IDENTIFIER = "Deeply_Customized_Device_Name_needs_to_be_queried";

	_pMappingObj = NULL;

}

CDeviceManager::~CDeviceManager() {
	// TODO Auto-generated destructor stub
}

void CDeviceManager::SetDeviceSocketMapping(CDeviceSocketMapping * pMappingObj)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_pMappingObj = pMappingObj;
}

void CDeviceManager::StartMonitoringDevices()
{

}

void CDeviceManager::SendCommand(const std::string& deviceName, const std::string& command)
{

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
			device.deviceName = p.getFileName();
			device.state = DeviceState::OPENED;
			_devices.push_back(device);
			devicesExist.push_back(true);
		}

		//notify unplugged device file
		for(auto i = (devicesExist.size() -1); i >= 0; i--)
		{
			//if an active device is missing, then it is unplugged.
			if(devicesExist[i] == false) {
				if(_devices[i].state == DeviceState::ACTIVE) {
					_pMappingObj->OnDeviceUnplugged(_devices[i].deviceName); //notify mapping of the unplug
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

void CDeviceManager::pollDevices()
{

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
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);
				if(_pMappingObj == NULL) {
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
