/*
 * CSocketManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CSocketManager.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

CSocketManager::CSocketManager():Task("SocketManager")
{
	_pMapping = NULL;
	_lastSocketId = STARTING_SOCKET_ID;
}

CSocketManager::~CSocketManager()
{
	// TODO Auto-generated destructor stub
}

void CSocketManager::SetDeviceSocketMapping(CDeviceSocketMapping * pMapping)
{
	_pMapping = pMapping;
}

void CSocketManager::OnDeviceReply(const long long socketId, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _sockets.find(socketId);
	if(it == _sockets.end()) {
		pLogger->LogError("CSocketManager::OnDeviceReply no socket for socketId: " + std::to_string(socketId));
	}
	else {
		pLogger->LogInfo("CSocketManager::OnDeviceReply send reply: " + std::to_string(socketId) + ":" + reply);
		sendReply(it->second, reply);
	}
}

void CSocketManager::OnDeviceUnplugged(const long long socketId)
{

}

void CSocketManager::OnNewDevice(const std::string& deviceName)
{

}

void CSocketManager::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_sockets[_lastSocketId++] = socket;
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

