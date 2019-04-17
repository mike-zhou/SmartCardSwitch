/*
 * CommandRunner.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: mikez
 */

#include "CommandRunner.h"
#include "Logger.h"

extern Logger * pLogger;

CommandRunner::CommandRunner(unsigned long lowClks, unsigned long highClks, const std::string deviceIp, unsigned int devicePort) : Task("iFinger")
{
	_lowClks = lowClks;
	_highClks = highClks;
	_deviceIp = deviceIp;
	_devicePort = devicePort;

	_socketConnected = false;
	_deviceConnected = false;
	_isPressing = false;

	_activeClientSocketIndex = -1;
}

void CommandRunner::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock lock(_mutex);

	if(_isPressing) {

	}
}

void CommandRunner::connectDevice()
{

}

void CommandRunner::pollClientSockets()
{

}

void CommandRunner::pollDeviceSocket()
{

}


void CommandRunner::runTask()
{
	pLogger->LogInfo("CommandRunner::runTask starts");

	while(1)
	{
		if(isCancelled()) {
			break;
		}

		try
		{
			if(!_socketConnected)
			{
				sleep(1000); //sleep 1 second

				Poco::Net::SocketAddress address(_deviceIp, _devicePort);
				Poco::Timespan span(1000000);// 1 second
				try
				{
					_deviceSocket.connect(address, span);
					_socketConnected = true;
				}
				catch(...)
				{
					pLogger->LogError("CommandRunner::runTask failed in connecting: " + address.toString());
				}
			}
			else
			{
				pollClientSockets();

				if(!_deviceConnected) {
					connectDevice();
				}
				else {
					pollDeviceSocket();
				}
			}
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError(("CommandRunner::runTask poco exception: " + e.displayText()));
		}
		catch(std::exception &e)
		{
			pLogger->LogError("CommandRunner::runTask std exception: " + std::string(e.what()));
		}
		catch(...)
		{
			pLogger->LogError("CommandRunner::runTask unknown exception");
		}
	}

	pLogger->LogInfo("CommandRunner::runTask exits");
}
