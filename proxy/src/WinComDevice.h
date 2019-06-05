#pragma once
#if defined(_WIN32) || defined(_WIN64)

#include "ILowlevelDevice.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"

class WinComDevice : public ILowlevelDevice
{
public:
	WinComDevice(const std::string & name, ILowlevelDeviceObsesrver * pObserver);
	virtual ~WinComDevice() {}

private:
	virtual void runTask() override;
	virtual bool SendCommand(const std::vector<unsigned char> & command, std::string & info) override;
	virtual void Disconnect() override;

private:
	WinComDevice();

	const unsigned int MAXIMUM_QUEUE_SIZE = 0xFFFF;

	const std::string DEVICE_NOT_CONNECTED = "device not connected";
	const std::string DEVICE_NOT_NORMAL = "device not normal";
	const std::string DEVICE_ERROR = "device error";
	const std::string DEVICE_NOT_ACCEPT_FURTHER_DATA = "device not accept further data";

	const int WRITING_TIMEOUT = 1000; // 1000 milliseconds
	const int READING_TIMEOUT = 10; // 10 milliseconds

	HANDLE _handle;
	bool _bExit;
	LowlevelDeviceState _state;
	std::deque<unsigned char> _inputQueue;
	std::deque<unsigned char> _outputQueue;
	Poco::Timestamp _timeLastWrite;

	void openDevice();
	bool receiveData();
	bool sendData();
};


#else
#endif

