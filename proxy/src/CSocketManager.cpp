/*
 * CSocketManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CSocketManager.h"

CSocketManager::CSocketManager():Task("SocketManager")
{
	_pDeviceSocketMappingObj = NULL;

}

CSocketManager::~CSocketManager()
{
	// TODO Auto-generated destructor stub
}

void CSocketManager::SetDeviceSocketMapping(CDeviceSocketMapping * pMapping)
{

}

void CSocketManager::OnDeviceReply(const long long socketId, const std::string& reply)
{

}

void CSocketManager::OnDeviceUnplugged(const long long socketId)
{

}

void CSocketManager::AddSocket(StreamSocket& socket)
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

