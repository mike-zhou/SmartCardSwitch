/*
 * Rs232.h
 *
 *  Created on: 8/09/2020
 *      Author: user1
 */

#ifndef LINUXRS232_H_
#define LINUXRS232_H_

#if defined(_WIN32) || defined(_WIN64)
#else

#include <string>
#include <deque>
#include <vector>

#include "Poco/Task.h"
#include "IDataExchange.h"

enum class DeviceState
{
	DeviceConnected,
	DeviceNormal,
	DeviceError,
	DeviceNotConnected
};

class LinuxRS232: public IDataExchange, public Poco::Task
{
public:
	LinuxRS232(const std::string devicePath);
	~LinuxRS232();
	void Send(const unsigned char * pData, const unsigned int amount) override;
	void Connect(IDataExchange * pInterface) override;

private:
	void runTask() override;

private:
	std::string _name;
	IDataExchange * _pPeer;

	static const unsigned int MAX_OUTPUT_QUEUE_SIZE = 0x10000;
	const unsigned int WRITING_TIMEOUT = 1; // 1 milliseconds
	const unsigned int READING_TIMEOUT = 10; // 10 milliseconds

	Poco::Mutex _mutex;

	int _fd;
	bool _bExit;
	DeviceState _state;
	unsigned char _inputBuffer[1024];
	std::deque<unsigned char> _inputQueue;
	std::deque<unsigned char> _outputQueue;
	unsigned char _dataBuffer[MAX_OUTPUT_QUEUE_SIZE];
	unsigned long _totalRead, _totalWrite;

	void openDevice();
	bool receiveData();
	bool sendData();
	void processReceivedData();
};


#endif // _WIN32 || _WIN64
#endif /* LINUXRS232_H_ */
