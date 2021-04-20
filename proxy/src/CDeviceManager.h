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
#include "Poco/TaskManager.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "IDevice.h"
#include "IDeviceObserver.h"
#include "ILowlevelDevice.h"
#include "CrcCcitt.h"
#include "CDataExchange.h"


/***************
 * Functionalities of this class object
 * 1. enumerate devices
 * 2. send command to devices
 * 3. notify observer/s of replies/events from device.
  ****************/
class CDeviceManager: public Poco::Task, public IDevice, public ILowlevelDeviceObsesrver
{
public:
	CDeviceManager();
	virtual ~CDeviceManager();

	void SetObserver(IDeviceObserver * pObserver);
	void AddDeviceFile(const std::string & deviceFilePath);

private:
	// Called by DeviceSocketMapping object to send a command to device.
	virtual void SendCommand(const std::string& deviceName, const std::string& command) override;

	virtual void onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info) override;
	virtual void onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice) override;
	virtual void onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data) override;

	void runTask();

private:
	const int MUTEX_TIMEOUT = 100; //100 milliseconds
	Poco::Mutex _mutex;
	std::string _lockMutexFor;

	const char ILLEGAL_CHARACTER_REPLACEMENT = '?';
	const char * COMMAND_QUERY_NAME = "C 1 0";
	const char COMMAND_TERMINATER = 0x0D; //carriage return

	enum DeviceState
	{
		CLOSED = 0,
		OPENED,
		CLEARING_BUFFER,
		BUFFER_CLEARED,
		RECEIVING_NAME,
		ACTIVE,
		DEVICE_ERROR
	};

	std::vector<std::string> _deviceFiles;

	struct Device
	{
		Device() {
			state = CLOSED;
		}

		static const Poco::Timestamp::TimeDiff FileReadWarningThreshold = 1000000; //1 second
		static const Poco::Timestamp::TimeDiff FileWriteWarningThreshold = 1000000; // 1 second

		enum DeviceState state;
		Poco::Timestamp bufferCleaningStamp;
		Poco::Timestamp readStamp;
		Poco::Timestamp writeStamp;

		std::string fileName; //name of device file
		std::string deviceName; //name queried from COMMAND_QUERY_NAME. Each device is supposed to have a unique name.
		std::deque<unsigned char> reply;

		CDataExchange dataExchange;
	};

	std::vector<struct Device> _devices;

	void lockMutex(const std::string & functionName, const std::string & purpose);
	void unlockMutex();

	void onReply(struct Device& device, const std::string& reply);
	void onDeviceCanBeRead(struct Device& device, std::deque<unsigned char> & reply);
	void onDeviceCanBeWritten(struct Device& device, ILowlevelDevice * pLowlevelDevice);
	void onDeviceError(struct Device& device, const std::string & errorInfo);
	void pollDevices();

	void enqueueCommand(struct Device& device, const char * pCommand);
	void enqueueCommand(struct Device& device, const std::string command);

	IDeviceObserver * _pObserver;
	Poco::TaskManager _tm;
};

#endif /* CDEVICEMANAGER_H_ */
