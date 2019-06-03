#pragma once

#include <string>
#include <deque>
#include <vector>

#include "Poco/Task.h"

enum class LowlevelDeviceState
{
	DeviceConnected,
	DeviceNormal,
	DeviceError,
	DeviceNotConnected
};

class ILowlevelDevice;

/**
Sub class implements member functions to receive device state and replies.
*/
class ILowlevelDeviceObsesrver
{
public:
	virtual ~ILowlevelDeviceObsesrver() {}

	virtual void onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data) = 0;
	virtual void onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice) = 0;
	virtual void onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info) = 0;
};

/**
sub class implements the run() in Poco::Task, exchange data with device specified by parameter 'name'.
*/
class ILowlevelDevice: public Poco::Task
{
public:
	ILowlevelDevice(const std::string & name, ILowlevelDeviceObsesrver * pObserver): Task(name)
	{
		_name = name;
		_pObserver = pObserver;
	}
	virtual ~ILowlevelDevice() {}

	virtual bool SendCommand(const std::vector<unsigned char> & command, std::string & info) = 0;
	virtual void Disconnect() = 0;

protected:
	std::string _name;
	ILowlevelDeviceObsesrver * _pObserver;
};
