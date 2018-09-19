/*
 * CDeviceSocketMapping.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CDeviceSocketMapping.h"

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

}

void CDeviceSocketMapping::OnDeviceExtracted(const std::string& deviceName)
{

}

void CDeviceSocketMapping::OnDeviceReply(const std::string& deviceName, const std::string& reply)
{

}


std::vector<std::string> CDeviceSocketMapping::GetDeviceNames()
{
	std::vector<std::string> devices;

	return devices;
}

bool CDeviceSocketMapping::Bond(const long long socketId, const std::string& deviceName)
{
	return false;
}

void CDeviceSocketMapping::SendCommand(const long long socketId, const std::string& command)
{

}

void CDeviceSocketMapping::OnSocketBroken(const long long socketId)
{

}
