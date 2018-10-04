/*
 * CDeviceSocketMapping.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CDeviceSocketMapping.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

CDeviceSocketMapping::CDeviceSocketMapping(CDeviceManager * pDeviceManager, CSocketManager * pSocketManager)
{
	_pDeviceManager = pDeviceManager;
	_pSocketManager = pSocketManager;

	_pDeviceManager->SetDeviceSocketMapping(this);
	_pSocketManager->SetDeviceSocketMapping(this);
}

CDeviceSocketMapping::~CDeviceSocketMapping()
{
	// TODO Auto-generated destructor stub
}

void CDeviceSocketMapping::OnDeviceInserted(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_deviceSocketMap.end() == _deviceSocketMap.find(deviceName)) {
		_deviceSocketMap[deviceName] = INVALID_SOCKET_ID;
		_pSocketManager->OnNewDevice(deviceName);
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceInserted: device has already existed: " + deviceName);
	}
}

void CDeviceSocketMapping::OnDeviceUnplugged(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto socketId = _deviceSocketMap[deviceName];

		pLogger->LogInfo("CDeviceSocketMapping::OnDeviceUnplugged: device is unplugged: " + deviceName);
		_deviceSocketMap.erase(it);

		if(socketId != INVALID_SOCKET_ID) {
			_pSocketManager->OnDeviceUnplugged(socketId);
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceUnplugged: unknown device is unplugged: " + deviceName);
	}
}

void CDeviceSocketMapping::OnDeviceReply(const std::string& deviceName, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto socketId = _deviceSocketMap[deviceName];

		pLogger->LogInfo("CDeviceSocketMapping::OnDeviceReply: " + deviceName + ":" + reply);
		if(socketId != INVALID_SOCKET_ID) {
			_pSocketManager->OnDeviceReply(socketId, reply);
		}
		else {
			pLogger->LogError("CDeviceSocketMapping::OnDeviceReply: none wants reply from device: " + deviceName);
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceReply: unknown device has a  reply: " + deviceName + ":" + reply);
	}
}


std::map<std::string, long long> CDeviceSocketMapping::GetMapping()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	return _deviceSocketMap;
}

bool CDeviceSocketMapping::Bond(const long long socketId, const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto sId = _deviceSocketMap[deviceName];

		if(sId != INVALID_SOCKET_ID) {
			pLogger->LogError("CDeviceSocketMapping::Bond: device has already been coupled with socket: " + deviceName + ":" + std::to_string(sId));
		}
		else {
			pLogger->LogInfo("CDeviceSocketMapping::Bond: " + deviceName + ":" + std::to_string(socketId));
			_deviceSocketMap[deviceName] = socketId;
			return true;
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::Bond: device is unplugged: " + deviceName);
	}

	return false;
}

void CDeviceSocketMapping::SendCommand(const long long socketId, const std::string& command)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.begin();
	for(; it != _deviceSocketMap.end(); it++)
	{
		if(it->second == socketId) {
			pLogger->LogInfo("CDeviceSocketMapping::SendCommand: " + it->first + ":" + std::to_string(it->second));
			_pDeviceManager->SendCommand(it->first, command);
			break;
		}
	}
	if(it == _deviceSocketMap.end()) {
		pLogger->LogError("CDeviceSocketMapping::SendCommand: no device for socketId: " + std::to_string(socketId));
	}
}

void CDeviceSocketMapping::OnSocketBroken(const long long socketId)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.begin();
	for(; it != _deviceSocketMap.end(); it++)
	{
		if(it->second == socketId) {
			pLogger->LogInfo("CDeviceSocketMapping::OnSocketBroken: " + it->first + ":" + std::to_string(it->second));
			it->second = INVALID_SOCKET_ID;
			break;
		}
	}
	if(it == _deviceSocketMap.end()) {
		pLogger->LogError("CDeviceSocketMapping::OnSocketBroken: no device for socketId: " + std::to_string(socketId));
	}
}
