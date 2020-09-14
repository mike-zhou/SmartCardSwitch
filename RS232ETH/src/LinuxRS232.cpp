/*
 * LinuxRS232.cpp
 *
 *  Created on: 8/09/2020
 *      Author: user1
 */

#if defined(_WIN32) || defined(_WIN64)
#else

#include <poll.h>
#include <stddef.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "Poco/File.h"

#include "LinuxRS232.h"
#include "Logger.h"

extern Logger * pLogger;


LinuxRS232::LinuxRS232(const std::string devicePath): Task("RS232")
{
	_state = DeviceState::DeviceNotConnected;
	_name = devicePath;
	_pPeer = nullptr;
	_fd = -1;
	_bExit = false;
	_totalRead = 0;
	_totalWrite = 0;
}

LinuxRS232::~LinuxRS232()
{

}

void LinuxRS232::openDevice()
{
	//open device
	_fd = open(_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_fd < 0) {
		auto errorNumber = errno;
		//cannot open device file
		pLogger->LogError("LinuxRS232::openDevice cannot open: " + _name + ", reason: " + std::string(strerror(errorNumber)));
		return;
	}

	//set device file
	{
		struct termios tios;
		int rc;
		//change settings of device.
		rc = tcgetattr(_fd, &tios);
		if (0 != rc)
		{
			auto e = errno;
			pLogger->LogError("LinuxRS232::openDevice tcgetattr errno: " + std::to_string(e));
			close(_fd);
			return;
		}
		rc = cfsetspeed(&tios, B115200);
		if (0 != rc)
		{
			auto e = errno;
			pLogger->LogError("LinuxRS232::openDevice cfsetspeed errno: " + std::to_string(e));
			close(_fd);
			return;
		}
#if 0
		//c_iflag
		tios.c_iflag &= ~ICRNL;
		tios.c_iflag &= ~IXON;
		tios.c_iflag |= IGNPAR;
		//c_oflag
		tios.c_oflag &= ~ONLCR;
		tios.c_oflag &= ~OPOST;
		//c_cflag
		tios.c_cflag &= ~CLOCAL;
		//					tios.c_cflag |= CS8;
		//					tios.c_cflag &= ~CSTOPB;
		//					tios.c_cflag &= ~PARENB;
							//c_lflag
		tios.c_lflag &= ~ISIG;
		tios.c_lflag &= ~ICANON;
		tios.c_lflag &= ~IEXTEN;
		tios.c_lflag &= ~ECHO;
		tios.c_lflag &= ~ECHOE;
		tios.c_lflag &= ~ECHOK;
		tios.c_lflag &= ~ECHOCTL;
		tios.c_lflag &= ~ECHOKE;
		tios.c_lflag &= ~FLUSHO;
		tios.c_lflag &= ~EXTPROC;
#endif

#if 1 //cfmakeraw
		cfmakeraw(&tios);
		//polling read.
		tios.c_cc[VMIN] = 0;
		tios.c_cc[VTIME] = 0;
		//8N1
		tios.c_cflag |= CS8;
		tios.c_cflag &= ~CSTOPB; //1 stop bit
		tios.c_cflag &= ~PARENB; //no parity
		//others
		tios.c_cflag |= CLOCAL; //ignore modem control lines
		tios.c_cflag |= CREAD; //enable receiver
		tios.c_cflag &= ~CRTSCTS; //no RTS/CTS flow control
#endif
		rc = tcsetattr(_fd, TCSANOW, &tios);
		if (0 != rc)
		{
			auto e = errno;
			pLogger->LogError("LinuxRS232::openDevice tcsetattr errno: " + std::to_string(e));
			close(_fd);
			return;
		}
	}

	pLogger->LogInfo("LinuxRS232::openDevice file is opened: " + _name);
	_state = DeviceState::DeviceNormal;
}

bool LinuxRS232::receiveData()
{
	pollfd desc;

	desc.fd = _fd;
	desc.events = POLLIN | POLLERR;
	desc.revents = 0;

	auto rc = poll(&desc, 1, READING_TIMEOUT);
	auto errorNumber = errno;

	if(rc == 0) {
		return true; //nothing to read
	}
	if(rc < 0)
	{
		pLogger->LogError("LinuxRS232::receiveData error in poll: " + _name + " errno: " + std::to_string(errorNumber));
		_state = DeviceState::DeviceError;
		return false;
	}
	if((desc.events & POLLERR) && (errorNumber > 0)) {
		pLogger->LogError("LinuxRS232::receiveData error in poll: " + _name + " errno: " + std::to_string(errorNumber));
		_state = DeviceState::DeviceError;
		return false;
	}

	auto amount = read(_fd, _inputBuffer, 1024);
	errorNumber = errno;

	if(amount < 0) {
		pLogger->LogError("LinuxRS232::receiveData failed in reading device: " + _name + " errno: " + std::to_string(errorNumber));
		_state = DeviceState::DeviceError;
		return false;
	}
	else if (amount == 0) {
		pLogger->LogError("LinuxRS232::receiveData no data is read from device: " + _name + " errno: " + std::to_string(errorNumber));
		_state = DeviceState::DeviceError;
		return false;
	}
	else if(amount > 0)
	{
		try
		{
			for(unsigned int i=0; i<amount; i++) {
				_inputQueue.push_back(_inputBuffer[i]);
			}
			_totalRead += amount;
			pLogger->LogInfo("LinuxRS232::receiveData received " + std::to_string(amount) + " bytes from " + _name + ", totally read: " + std::to_string(_totalRead));
		}
		catch(...)
		{
			pLogger->LogError("LinuxRS232::receiveData failed to save received data");
			_bExit = true;
			return false;
		}
	}

	return true;
}

bool LinuxRS232::sendData()
{
	int amount = 0;

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_outputQueue.empty()) {
		return true;
	}

	std::vector<unsigned char> data;
	for(unsigned int i=0; i<_outputQueue.size(); i++) {
		data.push_back(_outputQueue[i]);
	}

	//write data to device
	{
		pollfd desc;

		desc.fd = _fd;
		desc.events = POLLOUT | POLLERR;

		auto rc = poll(&desc, 1, WRITING_TIMEOUT);
		auto errorNumber = errno;

		if(rc == 0)
		{
			//timeout
			return true;
		}
		if(rc < 0)
		{
			pLogger->LogError("LinuxRS232::sendData error in polling: " + _name);
			_state = DeviceState::DeviceError;
			return false;
		}
		if((desc.events & POLLERR) && (errorNumber > 0)) {
			pLogger->LogError("LinuxRS232::sendData error in events: " + _name + " errno: " + std::to_string((int)errorNumber));
			_state = DeviceState::DeviceError;
			return false;
		}

		if(desc.events & POLLOUT)
		{
			amount = write(_fd, data.data(), data.size());
			errorNumber = errno;

			if(amount < 0)
			{
				pLogger->LogError("LinuxRS232::sendData error in sending: " + _name + " errno: " + std::to_string((int)errorNumber));
				_state = DeviceState::DeviceError;
				return false;
			}
			else if(amount > 0)
			{
				_totalWrite += amount;
				pLogger->LogInfo("LinuxRS232::sendData wrote " + std::to_string(amount) + " bytes to " + _name + ", totally write: " + std::to_string(_totalWrite));
			}
			else if(amount == 0)
			{
				pLogger->LogError("LinuxRS232::sendData failed in writing device: " + _name);
			}
		}
	}

	//remove data which has been sent.
	for(int i=0; i<amount; i++) {
		_outputQueue.pop_front();
	}

	return true;
}

void LinuxRS232::runTask()
{
	pLogger->LogInfo("LinuxRS232::runTask runs for: " + _name);

	for(;;)
	{
		if(isCancelled()) {
			break;
		}
		else if(_bExit) {
			break;
		}

		switch(_state)
		{
			case DeviceState::DeviceConnected:
			{
				sleep(1000);
				openDevice();
			}
			break;

			case DeviceState::DeviceNormal:
			{
				// _state is changed if any error in data exchange
				if(sendData()) {
					//no error in sendData(), then readData.
					receiveData();
				}
			}
			break;

			case DeviceState::DeviceError:
			{
				pLogger->LogError("LinuxRS232::runTask error happens in device: " + _name);
				close(_fd);
				_state = DeviceState::DeviceNotConnected;
			}
			break;

			case DeviceState::DeviceNotConnected:
			{
				Poco::File file(_name);

				if(file.exists()) {
					_state = DeviceState::DeviceConnected;
				}
				else {
					pLogger->LogError("LinuxRS232::runTask device not connect: " + _name);
					sleep(5000); //continue to check device existence 1 second later.
				}
			}
			break;

			default:
			{
				pLogger->LogError("LinuxRS232::runTask wrong device state in " + _name + " : " + std::to_string((int)_state));
				_bExit = true;
			}
			break;
		}

		processReceivedData();
	}

	pLogger->LogInfo("LinuxRS232::runTask for " + _name + " existed");
}

void LinuxRS232::Send(const unsigned char * pData, const unsigned int amount)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(amount == 0) {
		return;
	}
	if(pData == nullptr) {
		return;
	}
	if (_state != DeviceState::DeviceNormal) {
		return;
	}

	for (unsigned int i = 0; i < amount; i++) {
		_outputQueue.push_back(pData[i]);
	}
}

void LinuxRS232::processReceivedData()
{
	unsigned int amount;

	if(_inputQueue.empty()) {
		return;
	}
	if(_pPeer == nullptr) {
		return;
	}

	for(amount = 0; amount <MAX_OUTPUT_QUEUE_SIZE;) {
		_dataBuffer[amount] = _inputQueue[0];
		amount++;

		_inputQueue.pop_front();
		if(_inputQueue.empty()) {
			break;
		}
	}

	_pPeer->Send(_dataBuffer, amount);
}

void LinuxRS232::Connect (IDataExchange * pInterface)
{
	_pPeer = pInterface;
}

#endif


