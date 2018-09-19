/*
 * CDeviceManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CDeviceManager.h"

CDeviceManager::CDeviceManager() : Task("CDeviceManager")
{
	_pMappingObj = NULL;

}

CDeviceManager::~CDeviceManager() {
	// TODO Auto-generated destructor stub
}

void CDeviceManager::SetDeviceSocketMapping(CDeviceSocketMapping * pMappingObj)
{

}

void CDeviceManager::StartMonitoringDevices()
{

}

void CDeviceManager::SendCommand(const std::string& deviceName, const std::string& command)
{

}

void CDeviceManager::runTask()
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
