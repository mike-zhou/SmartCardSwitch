/*
 * CSocketManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CSocketManager.h"
#include "ProxyLogger.h"
#include "ReplyFactory.h"

extern ProxyLogger * pLogger;

CSocketManager::CSocketManager():Task("SocketManager")
{
	_pDevice = NULL;
	_lastSocketId = STARTING_SOCKET_ID;
}

CSocketManager::~CSocketManager()
{
	// TODO Auto-generated destructor stub
}

void CSocketManager::SetDevice(IDevice * pDevice)
{
	_pDevice = pDevice;
}

void CSocketManager::OnDeviceInserted(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_deviceSocketMap.end() == _deviceSocketMap.find(deviceName)) {
		pLogger->LogInfo("CSocketManager::OnDeviceInserted new device is available: " + deviceName);
		_deviceSocketMap[deviceName] = INVALID_SOCKET_ID;
	}
	else {
		pLogger->LogError("CSocketManager::OnDeviceInserted: device has already existed: " + deviceName);
	}
}

void CSocketManager::OnDeviceUnplugged(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto socketId = _deviceSocketMap[deviceName];

		pLogger->LogInfo("CSocketManager::OnDeviceUnplugged: device is unplugged: " + deviceName);
		_deviceSocketMap.erase(it);

		if(socketId != INVALID_SOCKET_ID) {
			onDeviceUnpluged(socketId);
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceUnplugged: unknown device is unplugged: " + deviceName);
	}
}

void CSocketManager::OnDeviceReply(const std::string& deviceName, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto socketId = _deviceSocketMap[deviceName];

		pLogger->LogInfo("CDeviceSocketMapping::OnDeviceReply: " + deviceName + ":" + reply);
		if(socketId != INVALID_SOCKET_ID) {
			onDeviceReply(socketId, reply);
		}
		else {
			pLogger->LogError("CDeviceSocketMapping::OnDeviceReply: none wants reply from device: " + deviceName);
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceReply: unknown device has a  reply: " + deviceName + ":" + reply);
	}

}

void CSocketManager::onDeviceUnpluged(long long socketId)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _sockets.begin();
	for(; it != _sockets.end(); it++)
	{
		if(it->socketId == socketId) {
			auto reply = ReplyFactory::EventDeviceUnplugged();
			for(auto c=reply.begin(); c!=reply.end(); c++) {
				it->replyBuffer.push_back(*c);
			}
			break;
		}
	}
	if(it == _sockets.end()) {
		pLogger->LogError("CSocketManager::onDeviceUnpluged no socket for socketId is found: " + std::to_string(socketId));
	}
}

void CSocketManager::onDeviceReply(long long socketId, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _sockets.begin();
	for(; it!=_sockets.end(); it++)
	{
		if(it->socketId == socketId)
		{
			auto formatedReply = ReplyFactory::Reply(reply);
			//append the reply to buffer
			for(auto c = formatedReply.begin(); c != formatedReply.end(); c++)
			{
				it->replyBuffer.push_back(*c);
			}
			break;
		}
	}
	if(it == _sockets.end()) {
		pLogger->LogError("CSocketManager::OnDeviceReply no socket for socketId: " + std::to_string(socketId));
	}
	else {
		pLogger->LogInfo("CSocketManager::OnDeviceReply send reply: " + std::to_string(socketId) + ":" + reply);
	}
}


void CSocketManager::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	struct SocketWrapper wrapper;

	wrapper.socket = socket;
	wrapper.socketId = newSocketId();

	_sockets.push_back(wrapper);
}

bool CSocketManager::sendReply(StreamSocket& socket, const std::string& reply)
{

}

void CSocketManager::onCommand(StreamSocket& socket, const std::string& command)
{

}

void CSocketManager::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			sleep(100);
		}
	}
}

