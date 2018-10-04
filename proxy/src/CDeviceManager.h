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

	const std::string DEVICE_FOLDER_PATH = "/dev/serial/by-id";
	const std::string IDENTIFIER = "Deeply_Customized_Device_Name_needs_to_be_queried";;
	const char ILLEGAL_CHARACTER_REPLACEMENT = '?';
	const char * COMMAND_QUERY_NAME = "C 1";
	const char COMMAND_TERMINATER = '\n';

	bool _startMonitoringDevices;

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
		std::string fileName; //device file name in Linux /dev

		//name queried from COMMAND_QUERY_NAME. Each device is supposed to have a unique name.
		std::string deviceName;
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
