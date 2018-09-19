/*
 * CDeviceManager.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CDEVICEMANAGER_H_
#define CDEVICEMANAGER_H_

#include "Poco/Task.h"
#include "Poco/Mutex.h"

class CDeviceSocketMapping;

class CDeviceManager: public Poco::Task {
public:
	CDeviceManager();
	virtual ~CDeviceManager();

	void SetDeviceSocketMapping(CDeviceSocketMapping * pMappingObj);
	void StartMonitoringDevices();
	// Called by DeviceSocketMapping object to send a command to device.
	void SendCommand(const std::string& deviceName, const std::string& command);

	void runTask();

private:
	Poco::Mutex _mutex;

	CDeviceSocketMapping * _pMappingObj;
};

#endif /* CDEVICEMANAGER_H_ */
