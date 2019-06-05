
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

#include "LinuxComDevice.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

LinuxComDevice::LinuxComDevice(const std::string & name, ILowlevelDeviceObsesrver * pObserver)
	:ILowlevelDevice(name, pObserver)
{
	_fd = -1;
	_bExit = false;
	_state = LowlevelDeviceState::DeviceNotConnected;
}

LinuxComDevice::~LinuxComDevice()
{

}

void LinuxComDevice::openDevice()
{
	//open device
	_fd = open(_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (_fd < 0) {
		auto errorNumber = errno;
		//cannot open device file
		pLogger->LogError("LinuxComDevice::openDevice cannot open: " + _name + ", reason: " + std::string(strerror(errorNumber)));
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
			pLogger->LogError("LinuxComDevice::openDevice tcgetattr errno: " + std::to_string(e));
			close(_fd);
			return;
		}
		rc = cfsetspeed(&tios, B115200);
		if (0 != rc)
		{
			auto e = errno;
			pLogger->LogError("LinuxComDevice::openDevice cfsetspeed errno: " + std::to_string(e));
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
			pLogger->LogError("LinuxComDevice::openDevice tcsetattr errno: " + std::to_string(e));
			close(_fd);
			return;
		}
	}

	pLogger->LogInfo("LinuxComDevice::openDevice file is opened: " + _name);
	_state = LowlevelDeviceState::DeviceNormal;
}

bool LinuxComDevice::receiveData()
{
	pollfd desc;

	desc.fd = _fd;
	desc.events = POLLIN | POLLERR;
	desc.revents = 0;

	if(!_inputQueue.empty()) {
		_pObserver->onLowlevelDeviceReply(_name, _inputQueue);
	}

	auto rc = poll(&desc, 1, READING_TIMEOUT);
	auto errorNumber = errno;

	if(rc == 0) {
		return true; //nothing to read
	}
	if(rc < 0)
	{
		pLogger->LogError("LinuxComDevice::receiveData error in poll: " + _name + "errno: " + std::to_string(errorNumber));
		_state = LowlevelDeviceState::DeviceError;
		return false;
	}
	if(desc.events & POLLERR) {
		pLogger->LogError("LinuxComDevice::receiveData error in poll: " + _name + "errno: " + std::to_string(errorNumber));
		_state = LowlevelDeviceState::DeviceError;
		return false;
	}

	auto amount = read(_fd, _inputBuffer, 1024);
	errorNumber = errno;

	if(amount < 0) {
		pLogger->LogError("LinuxComDevice::receiveData failed in reading device: " + _name + " errno: " + std::to_string(errorNumber));
		_state = LowlevelDeviceState::DeviceError;
	}
	else if (amount == 0) {
		pLogger->LogError("LinuxComDevice::receiveData no data is read from device: " + _name + " errno: " + std::to_string(errorNumber));
	}
	else if(amount > 0)
	{
		std::string content;

		for(unsigned int i=0; i<amount; i++) {
			_inputQueue.push_back(_inputBuffer[i]);
			content.push_back((_inputBuffer[i]));
		}
		pLogger->LogInfo("LinuxComDevice::receiveData received " + std::to_string(amount) + " bytes");
		pLogger->LogInfo("LinuxComDevice::receiveData << " + content);

		_pObserver->onLowlevelDeviceReply(_name, _inputQueue);
	}

	return true;
}

bool LinuxComDevice::sendData()
{
	int amount = 0;

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_outputQueue.empty()) {
		_pObserver->onLowlevelDeviceWritable(_name, this); //ask for data from observer
	}

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
			pLogger->LogError("LinuxComDevice::sendData device not respond: " + _name);
			_state = LowlevelDeviceState::DeviceError;
			return false;
		}
		if(rc < 0)
		{
			pLogger->LogError("LinuxComDevice::sendData error in polling: " + _name);
			_state = LowlevelDeviceState::DeviceError;
			return false;
		}
		if((desc.events & POLLERR) && (errorNumber > 0)) {
			pLogger->LogError("LinuxComDevice::sendData error in events: " + _name + " errno: " + std::to_string((int)errorNumber));
			_state = LowlevelDeviceState::DeviceError;
			return false;
		}

		if(desc.events & POLLOUT)
		{
			amount = write(_fd, data.data(), data.size());
			errorNumber = errno;

			if(amount < 0)
			{
				pLogger->LogError("LinuxComDevice::sendData error in sending: " + _name + " errno: " + std::to_string((int)errorNumber));
				_state = LowlevelDeviceState::DeviceError;
				return false;
			}
			else if(amount > 0)
			{
				std::string info;
				pLogger->LogInfo("LinuxComDevice::sendData wrote " + std::to_string(amount) + " bytes to " + _name);
				for(int i=0; i<amount; i++) {
					info.push_back(data[i]);
				}
				pLogger->LogInfo("LinuxComDevice::sendData >> " + info);
			}
			else if(amount == 0)
			{
				pLogger->LogError("LinuxComDevice::sendData failed in writing device: " + _name);
			}
		}
	}

	//remove data which has been sent.
	for(int i=0; i<amount; i++) {
		_outputQueue.pop_front();
	}

	return true;
}

void LinuxComDevice::runTask()
{
	pLogger->LogInfo("LinuxComDevice::runTask runs for: " + _name);

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
			case LowlevelDeviceState::DeviceConnected:
			{
				sleep(1000);

				openDevice();
				if(_state == LowlevelDeviceState::DeviceNormal) {
					_pObserver->onLowlevelDeviceState(_name, _state, _name + " is opened successfully");
				}
				else {
					_state = LowlevelDeviceState::DeviceNotConnected;
				}
			}
			break;

			case LowlevelDeviceState::DeviceNormal:
			{
				// _state is changed if any error in data exchange
				if(sendData()) {
					//no error in sendData(), then readData.
					receiveData();
				}
			}
			break;

			case LowlevelDeviceState::DeviceError:
			{
				pLogger->LogError("LinuxComDevice::runTask device error: " + _name);
				_pObserver->onLowlevelDeviceState(_name, _state, "");
				_bExit = true;
			}
			break;

			case LowlevelDeviceState::DeviceNotConnected:
			{
				Poco::File file(_name);

				if(file.exists()) {
					_state = LowlevelDeviceState::DeviceConnected;
					_pObserver->onLowlevelDeviceState(_name, _state, _name + " connected");
				}
				else {
					_pObserver->onLowlevelDeviceState(_name, _state, _name + " not connected");
					sleep(1000); //continue to check device existence 1 second later.
				}
			}
			break;

			default:
			{
				pLogger->LogError("LinuxComDevice::runTask wrong device state in " + _name + " : " + std::to_string((int)_state));
				_bExit = true;
			}
			break;
		}
	}

	pLogger->LogInfo("LinuxComDevice::runTask for " + _name + " existed");
}

bool LinuxComDevice::SendCommand(const std::vector<unsigned char> & command, std::string & info)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	info.clear();

	if(command.empty()) {
		info = EMPTY_COMMAND;
		return false;
	}
	if (_state == LowlevelDeviceState::DeviceNotConnected) {
		info = DEVICE_NOT_CONNECTED;
		return false;
	}
	if (_state == LowlevelDeviceState::DeviceError) {
		info = DEVICE_ERROR;
		return false;
	}
	if (_state != LowlevelDeviceState::DeviceNormal) {
		info = DEVICE_NOT_NORMAL;
		return false;
	}
	if(_outputQueue.size() > MAX_OUTPUT_QUEUE_SIZE) {
		info = DEVICE_NOT_ACCEPT_FURTHER_DATA;
		return false;
	}

	for (unsigned int i = 0; i < command.size(); i++) {
		_outputQueue.push_back(command[i]);
	}

	return true;
}

void LinuxComDevice::Disconnect()
{
	_bExit = true;
}

#endif
