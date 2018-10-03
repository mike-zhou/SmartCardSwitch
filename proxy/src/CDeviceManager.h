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
#include <deque>
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
	const char ILLEGAL_CHARACTER_REPRESENTIVE = '?';
	const char * COMMAND_QUERY_NAME = "C 1\n";

	enum DeviceState
	{
		CLOSED = 0,
		OPENED,
		RECEIVING_NAME,
		ACTIVE,
		ERROR
	};

	struct Device
	{
		enum DeviceState state;

		int fd;
		std::string fileName; //device file name

		std::string deviceName; //name queried from COMMAND_QUERY_NAME
		std::deque<char> outgoing;
		std::deque<char> incoming;
	};

	std::vector<struct Device> _devices;

	//check if any DCD device is inserted or unpluged.
	void checkDevices();

	void onReply(struct Device& device, const std::string& reply);
	void onDeviceInput(struct Device& device);
	void onDeviceOutput(struct Device& device);
	void onDeviceError(struct Device& device);
	void pollDevices();

	void enqueueCommand(struct Device& device, const char * pCommand);
	void enqueueCommand(struct Device& device, const std::string command);

	CDeviceSocketMapping * _pMapping;
};

#endif /* CDEVICEMANAGER_H_ */
