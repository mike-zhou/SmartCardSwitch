/*
 * CDeviceSocketMapping.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CDEVICESOCKETMAPPING_H_
#define CDEVICESOCKETMAPPING_H_

#include <vector>
#include <map>
#include "Poco/Mutex.h"

#include "CDeviceManager.h"
#include "CSocketManager.h"

class CDeviceSocketMapping {
public:
	CDeviceSocketMapping(CDeviceManager * pDeviceManager, CSocketManager * pSocketManager);
	virtual ~CDeviceSocketMapping();

	////////////////////////////////////////////
	// interface with device manager.
	////////////////////////////////////////////
	// called by DeviceManager when a device is inserted.
	void OnDeviceInserted(const std::string& deviceName);
	// called by Device Manager when a device is unplugged.
	void OnDeviceExtracted(const std::string& deviceName);
	// called by Device Manager when a device has some replies.
	void OnDeviceReply(const std::string& deviceName, const std::string& reply);

	/////////////////////////////////////////
	//interface with socket manager
	/////////////////////////////////////////
	// return a vector of device names
	std::vector<std::string> GetDeviceNames();
	// called by SocketManager when a socket bonds to a device
	bool Bond(const long long socketId, const std::string& deviceName);
	// called by SocketManager when a socket sends a command to device
	void SendCommand(const long long socketId, const std::string& command);
	// called by SocketManager when a socket connection is terminated.
	void OnSocketBroken(const long long socketId);

private:
	Poco::Mutex _mutex;

	std::map<std::string, int> _deviceSocketMap;
	CDeviceManager * _pDeviceManager;
	CSocketManager * _pSocketManager;
};

#endif /* CDEVICESOCKETMAPPING_H_ */
