#pragma once

#if defined(_WIN32) || defined(_WIN64)
#else

#include "ILowlevelDevice.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"

class LinuxComDevice : public ILowlevelDevice
{
public:
	LinuxComDevice(const std::string & name, ILowlevelDeviceObsesrver * pObserver);
	virtual ~LinuxComDevice();

private:
	virtual void runTask() override;
	virtual bool SendData(const std::vector<unsigned char> & command, std::string & info) override;
	virtual void Disconnect() override;

private:
	LinuxComDevice();

	const unsigned int MAX_OUTPUT_QUEUE_SIZE = 1024;
	const std::string DEVICE_NOT_CONNECTED = "device not connected";
	const std::string DEVICE_NOT_NORMAL = "device not normal";
	const std::string DEVICE_ERROR = "device error";
	const std::string DEVICE_NOT_ACCEPT_FURTHER_DATA = "device not accept further data";
	const std::string EMPTY_COMMAND = "empty command";

	const unsigned int WRITING_TIMEOUT = 50; // 50 milliseconds
	const unsigned int READING_TIMEOUT = 10; // 10 milliseconds

	Poco::Mutex _mutex;

	int _fd;
	bool _bExit;
	LowlevelDeviceState _state;
	unsigned char _inputBuffer[1024];
	std::deque<unsigned char> _inputQueue;
	std::deque<unsigned char> _outputQueue;

	void openDevice();
	bool receiveData();
	bool sendData();
};

#endif
