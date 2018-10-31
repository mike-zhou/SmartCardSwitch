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
#include "Poco/Timestamp.h"
#include "IDevice.h"
#include "IDeviceObserver.h"


/***************
 * Functionalities of this class object
 * 1. enumerate devices
 * 2. send command to devices
 * 3. notify observer/s of replies/events from device.
  ****************/
class CDeviceManager: public Poco::Task, public IDevice
{
public:
	CDeviceManager();
	virtual ~CDeviceManager();

	void SetObserver(IDeviceObserver * pObserver);
	void StartMonitoringDevices();

	// Called by DeviceSocketMapping object to send a command to device.
	virtual void SendCommand(const std::string& deviceName, const std::string& command) override;

	void runTask();

private:
	Poco::Mutex _mutex;

	const std::string DEVICE_FOLDER_PATH = "/dev/serial/by-id";
	const std::string IDENTIFIER = "Deeply_Customized_Device_Name_needs_to_be_queried";
	const char ILLEGAL_CHARACTER_REPLACEMENT = '?';
	const char * COMMAND_QUERY_NAME = "C 1 0";
	const char COMMAND_TERMINATER = 0x0D; //carriage return

	bool _startMonitoringDevices;

	enum DeviceState
	{
		CLOSED = 0,
		OPENED,
		CLEARING_BUFFER,
		BUFFER_CLEARED,
		RECEIVING_NAME,
		ACTIVE,
		ERROR
	};

	struct Device
	{
		enum DeviceState state;
		Poco::Timestamp timeStamp;

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

	IDeviceObserver * _pObserver;
};

#endif /* CDEVICEMANAGER_H_ */
