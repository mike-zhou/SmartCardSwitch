/*
 * CDeviceManager.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CDEVICEMANAGER_H_
#define CDEVICEMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include "Poco/Task.h"
#include "Poco/Mutex.h"

class CDeviceSocketMapping;

class CDeviceManager: public Poco::Task
{
public:
	CDeviceManager();
	virtual ~CDeviceManager();

	void SetDeviceSocketMapping(CDeviceSocketMapping * pMappingObj);
	void StartMonitoringDevices();
	// Called by DeviceSocketMapping object to send a command to device.
	void SendCommand(const std::string& deviceName, const std::string& command);

	void runTask();

private:
	Poco::Mutex _mutex;

	std::string DEVICE_FOLDER_PATH;
	std::string IDENTIFIER;

	enum DeviceState
	{
		CLOSED = 0,
		OPENED,
		QUERYING_NAME,
		RECEIVING_NAME,
		ACTIVE,
		ERROR
	};

	struct Device
	{
		enum DeviceState state;

		int fd;
		std::string fileName;

		std::string deviceName;
		std::vector<char> outgoing;
		std::vector<char> incoming;
	};

	std::vector<struct Device> _devices;

	void checkDevices();
	void pollDevices();

	CDeviceSocketMapping * _pMappingObj;
};

#endif /* CDEVICEMANAGER_H_ */
