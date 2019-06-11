#if defined(_WIN32) || defined(_WIN64)

#include "Poco/File.h"

#include "WinComDevice.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

WinComDevice::WinComDevice(const std::string & name, ILowlevelDeviceObsesrver * pObserver)
	:ILowlevelDevice(name, pObserver)
{
	_handle = INVALID_HANDLE_VALUE;
	_bExit = false;
	_state = LowlevelDeviceState::DeviceNotConnected;
}

void WinComDevice::runTask()
{
	pLogger->LogInfo("WinComDevice::runTask runs for: " + _name);

	for (;;)
	{
		if (isCancelled()) {
			break;
		}
		else if (_bExit) {
			break;
		}

		switch (_state)
		{
		case LowlevelDeviceState::DeviceConnected:
		{
			sleep(1000);

			openDevice();
			if (_state == LowlevelDeviceState::DeviceNormal) {
				_pObserver->onLowlevelDeviceState(_name, _state, "");
			}
			else {
				_state = LowlevelDeviceState::DeviceNotConnected;
			}
		}
		break;

		case LowlevelDeviceState::DeviceNormal:
		{
			// _state is changed if any error in data exchange
			if (sendData()) {
				//no error in sendData(), then readData.
				receiveData();
			}
		}
		break;

		case LowlevelDeviceState::DeviceError:
		{
			pLogger->LogError("WinComDevice::runTask device error: " + _name);
			_pObserver->onLowlevelDeviceState(_name, _state, "");
			_bExit = true;
		}
		break;

		case LowlevelDeviceState::DeviceNotConnected:
		{
			Poco::File file(_name);

			if (file.exists()) {
				_state = LowlevelDeviceState::DeviceConnected;
				_pObserver->onLowlevelDeviceState(_name, _state, _name + " connected");
			}
			else {
				_pObserver->onLowlevelDeviceState(_name, _state, _name + " not connected");
				sleep(1000);
			}
		}
		break;

		default:
		{
			pLogger->LogError("WinComDevice::runTask wrong device state in " + _name + " : " + std::to_string((int)_state));
			_bExit = true;
		}
		break;
		}
	}

	pLogger->LogInfo("WinComDevice::runTask for " + _name + " existed");
}

bool WinComDevice::SendCommand(const std::vector<unsigned char> & command, std::string & info)
{
	info.clear();
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
	if (!_outputQueue.empty()) {
		info = DEVICE_NOT_ACCEPT_FURTHER_DATA;
		return false;
	}

	for (unsigned int i = 0; i < command.size(); i++) {
		_outputQueue.push_back(command[i]);
	}

	return true;
}

void WinComDevice::Disconnect()
{
	_bExit = true;
	
}

void WinComDevice::openDevice()
{
	std::string modeStr;

	_handle = CreateFileA(_name.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,                          /* no share  */
		NULL,                       /* no security */
		OPEN_EXISTING,
		0,                          /* no threads */
		NULL);                      /* no templates */

	if (_handle == INVALID_HANDLE_VALUE) {
		pLogger->LogError("WinComDevice::openDevice failed to open: " + _name);
		return;
	}

	modeStr = "baud=115200 data=8 parity=n stop=1 dtr=off rts=off";

	DCB port_settings;
	memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
	port_settings.DCBlength = sizeof(port_settings);

	if (!BuildCommDCBA(modeStr.c_str(), &port_settings))
	{
		pLogger->LogError("WinComDevice::openDevice unable to set comport dcb settings");
		CloseHandle(_handle);
		return;
	}

	if (!SetCommState(_handle, &port_settings))
	{
		pLogger->LogError("WinComDevice::openDevice unable to set comport cfg settings");
		CloseHandle(_handle);
		return;
	}

	COMMTIMEOUTS Cptimeouts;

	Cptimeouts.ReadIntervalTimeout = 0;
	Cptimeouts.ReadTotalTimeoutMultiplier = 0;
	Cptimeouts.ReadTotalTimeoutConstant = 10; //10 milliseconds
	Cptimeouts.WriteTotalTimeoutMultiplier = 0;
	Cptimeouts.WriteTotalTimeoutConstant = 50; //50 milliseconds

	if (!SetCommTimeouts(_handle, &Cptimeouts))
	{
		pLogger->LogError("WinComDevice::openDevice unable to set comport time-out settings");
		CloseHandle(_handle);
		return;
	}

	pLogger->LogInfo("WinComDevice::openDevice file is opened: " + _name);
	_timeLastWrite.update();
	_state = LowlevelDeviceState::DeviceNormal;
}

bool WinComDevice::receiveData()
{
	unsigned char buffer[1024];
	DWORD amount;

	//read until timeout
	for (;;)
	{
		amount = 0;
		auto rc = ReadFile(_handle, buffer, 1024, &amount, NULL);
		if (rc)
		{
			if (amount > 0)
			{
				std::string content;

				pLogger->LogInfo("WinComDevice::receiveData received " + std::to_string(amount) + " bytes from " + _name);
				for (unsigned int i = 0; i < amount; i++) {
					content.push_back(buffer[i]);
					_inputQueue.push_back(buffer[i]);
				}
				pLogger->LogInfo("WinComDevice::receiveData << " + content);
			}
			else
			{
				break;
			}
		}
		else
		{
			auto errorCode = GetLastError();
			pLogger->LogError("WinComDevice::receiveData error code: " + std::to_string(errorCode));
			break;
		}
	}

	if (!_inputQueue.empty()) {
		//notify observer of the remaining data.
		_pObserver->onLowlevelDeviceReply(_name, _inputQueue);
	}

	return true;
}

bool WinComDevice::sendData()
{
	DWORD amount;

	if (_outputQueue.empty()) {
		_pObserver->onLowlevelDeviceWritable(_name, this); //ask for data from observer
	}

	if (_outputQueue.empty()) {
		return true;
	}

	std::vector<unsigned char> data;
	for (unsigned int i = 0; i < _outputQueue.size(); i++) {
		data.push_back(_outputQueue[i]);
	}

	//write data to device
	auto rc = WriteFile(_handle, data.data(), data.size(), &amount, NULL);
	if (rc)
	{
		std::string content;
		pLogger->LogInfo("WinComDevice::sendData wrote " + std::to_string(amount) + " bytes to " + _name);
		for (unsigned int i = 0; i < amount; i++) 
		{
			unsigned char c = _outputQueue[0];
			content.push_back(c);
			_outputQueue.pop_front();
		}
		pLogger->LogInfo("WinComDevice::sendData >> " + content);
	}
	else
	{
		auto errorCode = GetLastError();
		pLogger->LogError("WinComDevice::sendData error code: " + std::to_string(errorCode) + " to " + _name);
		if (_outputQueue.size() > MAXIMUM_QUEUE_SIZE) {
			pLogger->LogError("WinComDevice::sendData discard " + std::to_string(_outputQueue.size()) + " bytes");
			_outputQueue.clear();
		}
	}

	return true;
}

#else
#endif
