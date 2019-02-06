/*
 * DeviceAccessor.cpp
 *
 *  Created on: Oct 13, 2018
 *      Author: user1
 */

#include "DeviceAccessor.h"
#include "Logger.h"
#include "MsgPackager.h"
#include "Poco/Net/NetException.h"

extern Logger * pLogger;

const char * DeviceAccessor::MSG_DEVICE_DISCONNECTED = "{\"event\":\"device disconnected\"}";

DeviceAccessor::DeviceAccessor() : Task("DeviceAccessor")
{
	_connected = false;
}

bool DeviceAccessor::Init(const Poco::Net::SocketAddress& deviceAddress)
{
	try
	{
		_socketAddress = deviceAddress;

		pLogger->LogInfo("DeviceAccessor::Init connecting to " + _socketAddress.toString());
		_socket.connect(_socketAddress);
		pLogger->LogInfo("DeviceAccessor::Init connected to " + _socketAddress.toString());

		_connected = true;
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("DeviceAccessor::Init exception " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("DeviceAccessor::Init unknown exception");
	}

	return _connected;
}

bool DeviceAccessor::ReConnect()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	if(_connected) {
		return true;
	}

	try
	{
		pLogger->LogInfo("DeviceAccessor::ReConnect: connecting to " + _socketAddress.toString());
		_socket.connect(_socketAddress);
		pLogger->LogInfo("DeviceAccessor::ReConnect: connected to " + _socketAddress.toString());

		_connected = true;
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("DeviceAccessor::ReConnect exception " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("DeviceAccessor::ReConnect unknown exception");
	}

	return _connected;
}

bool DeviceAccessor::SendCommand(const std::string& cmd)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(!_connected) {
		pLogger->LogError("DeviceAccessor::SendCommand not connected to device proxy");
		return false;
	}

	std::vector<unsigned char> pkg;
	MsgPackager::PackageMsg(cmd, pkg);

	for(auto it=pkg.begin(); it!=pkg.end(); it++) {
		_outgoing.push_back(*it);
	}

	pLogger->LogDebug("DeviceAccessor::SendCommand send " + cmd);
	return true;
}

void DeviceAccessor::AddObserver(IDeviceObserver * pObserver)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto observerIt = _observerPtrArray.begin();
	for(; observerIt!= _observerPtrArray.end(); observerIt++) {
		if(*observerIt == pObserver) {
			break;
		}
	}
	if(observerIt == _observerPtrArray.end())
	{
		//new observer
		_observerPtrArray.push_back(pObserver);
	}
	else
	{
		pLogger->LogError("DeviceAccessor::AddObserver duplicated observer");
	}
}

void DeviceAccessor::onIncoming()
{
	std::vector<std::string> jsons;

	MsgPackager::RetrieveMsgs(_incoming, jsons);

	//notify observers
	for(auto it=jsons.begin(); it!=jsons.end(); it++) {
		pLogger->LogDebug("DeviceAccessor::onIncoming notify observer of " + (*it));
		for(auto observerIt=_observerPtrArray.begin(); observerIt!=_observerPtrArray.end(); observerIt++) {
			(*observerIt)->OnFeedback(*it);
		}
	}
}

void DeviceAccessor::runTask()
{
	const int bufferLength = 1024;
	unsigned char buffer[bufferLength];
	Poco::Timespan timeSpan(10000); //10 ms
	Poco::Timespan zeroSpan;

	while(1)
	{
		if(isCancelled()) {
			_socket.close();
			break;
		}
		else
		{
			if(!_connected)
			{
				sleep(1000);
				if(!ReConnect()) {
					continue;
				}
			}

			//send out _outgoing to device
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);

				if(_outgoing.size() > 0)
				{
					if(_socket.poll(zeroSpan, Poco::Net::Socket::SELECT_WRITE))
					{
						int amount = 0;

						for(auto it=_outgoing.begin(); it!=_outgoing.end(); it++) {
							buffer[amount] = *it;
							amount++;
							if(amount >= bufferLength) {
								break;
							}
						}
						amount = _socket.sendBytes(buffer, amount, 0);
						if(amount > 0) {
							pLogger->LogInfo("DeviceAccessor::runTask byte amount sent out: " + std::to_string(amount));
							//delete the data which was sent out.
							for(; amount>0; amount--) {
								_outgoing.pop_front();
							}
						}
						else {
							pLogger->LogError("DeviceAccessor::runTask failed sending: " + std::to_string(amount));
							_outgoing.clear();
						}
					}
				}
			}

			//read device's feedback
			if(_socket.poll(timeSpan, Poco::Net::Socket::SELECT_READ))
			{
				int amount = 0;
				bool errorOccur = false;

				try {
					amount = _socket.receiveBytes(buffer, bufferLength, 0);
					if(amount <= 0) {
						//peer socket has shut down
						errorOccur = true;
						pLogger->Log("DeviceAccessor::runTask peer socket shut down: " + std::to_string(amount));
					}
					else {
						//save content read to incoming queue
						for(int i=0; i<amount; i++) {
							_incoming.push_back(buffer[i]);
						}
						//process incoming data
						onIncoming();
					}
				}
				catch(Poco::TimeoutException& e) {
					; // timeout exception can be ignored.
				}
				catch(Poco::Net::NetException& e) {
					errorOccur = true;
					pLogger->LogError("DeviceAccessor::runTask exception " + e.displayText());
				}
				catch(...) {
					errorOccur = true;
					pLogger->LogError("DeviceAccessor::runTask unknown exception");
				}

				if(errorOccur)
				{
					pLogger->LogInfo("DeviceAccessor::runTask disconnect from " + _socketAddress.toString());
					_socket.close();
					_connected = false;
					_incoming.clear();

					//notify observers of device disconnection
					std::string disconnection(MSG_DEVICE_DISCONNECTED);
					for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
						(*it)->OnFeedback(disconnection);
					}
				}
			}
		}
	}

	pLogger->LogInfo("DeviceAccessor::runTask exited");
}
