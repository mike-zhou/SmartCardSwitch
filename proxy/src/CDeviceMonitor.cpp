/*
 * CDeviceMonitor.cpp
 *
 *  Created on: Mar 13, 2019
 *      Author: mikez
 */

#if defined(_WIN32) || defined(_WIN64)
#else
#include "LinuxComDevice.h"
#endif

#include "ProxyLogger.h"
#include "CDeviceMonitor.h"

extern ProxyLogger * pLogger;

CDeviceMonitor::CDeviceMonitor(const std::string& filePath): Task("CDeviceMonitor")
{
	_deviceFile = filePath;
}

void CDeviceMonitor::onMonitorCanBeRead(std::deque<unsigned char> & reply)
{
	memset(_buffer, 0, BUFFER_LENGTH);

	int amount = 0;
	for(unsigned int i=0; (i<BUFFER_LENGTH) && (!reply.empty()); i++) {
		_buffer[i] = reply[0];
		reply.pop_front();
		amount++;
	}

	if(amount == 0) {
		pLogger->LogError("CDeviceMonitor::onMonitorCanBeRead ERROR: EOF is returned");
	}
	else
	{
		pLogger->LogInfo("CDeviceMonitor::onMonitorCanBeRead " + std::to_string(amount) + " bytes from: " + _deviceFile);
//		for(int i=0; i<amount; i++)
//		{
//			char buf[32];
//
//			sprintf(buf, "%02x,", _buffer[i]);
//			outputStr = outputStr + std::string(buf);
//		}
//		pLogger->LogInfo("CDeviceMonitor::onMonitorCanBeRead hex content: " + outputStr);
		pLogger->LogInfo("CDeviceMonitor::onMonitorCanBeRead char content: " + std::string((char *)_buffer));
	}
}

void CDeviceMonitor::onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info)
{
	if(state == LowlevelDeviceState::DeviceError) {
		pLogger->LogError("CDeviceMonitor::onLowlevelDeviceState low level device error");
	}
}

void CDeviceMonitor::onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice)
{

}

void CDeviceMonitor::onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data)
{
	if(deviceName == _deviceFile) {
		onMonitorCanBeRead(data);
	}
	else {
		pLogger->LogError("CDeviceMonitor::onLowlevelDeviceReply wrong monitor name: " + deviceName);
	}
}


void CDeviceMonitor::runTask()
{
	bool monitorOpened = false;

	pLogger->LogInfo("CDeviceMonitor::runTask start");

	if(_deviceFile.empty()) {
		pLogger->LogError("CDeviceMonitor::runTask no monitor device file set");
	}
	else
	{
		ILowlevelDevice * pLowlevelDevice = nullptr;

#if defined(_WIN32) || defined(_WIN64)
#else
		pLowlevelDevice = new LinuxComDevice(_deviceFile, this);
#endif

		if(pLowlevelDevice == nullptr) {
			pLogger->LogError("CDeviceMonitor::runTask() failed to launch: " + _deviceFile);
		}
		else {
			_tm.start(pLowlevelDevice);
			monitorOpened =  true;
		}
	}

	if(monitorOpened)
	{
		while(1)
		{
			if(isCancelled()) {
				break;
			}
			else
			{
				sleep(10);
			}
		}

		pLogger->LogInfo("CDeviceMonitor::runTask() stopping low level device...");
		_tm.cancelAll();
		_tm.joinAll();
	}

	pLogger->LogInfo("CDeviceMonitor::runTask exited");
}
