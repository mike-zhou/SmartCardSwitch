/*
 * CSocketManager.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CSOCKETMANAGER_H_
#define CSOCKETMANAGER_H_

#include <map>
#include <vector>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Net/StreamSocket.h"

using Poco::Net::StreamSocket;

class CDeviceSocketMapping;

class CSocketManager : public Poco::Task {
public:
	CSocketManager();
	virtual ~CSocketManager();

	void SetDeviceSocketMapping(CDeviceSocketMapping * pMapping);
	void OnDeviceReply(const long long socketId, const std::string& reply);
	void OnDeviceUnplugged(const long long socketId);

	void AddSocket(StreamSocket& socket);

	void runTask();

private:
	Poco::Mutex _mutex;

	long long _lastSocketId;
	std::map<std::string, long long> _deviceSocketMap;
	CDeviceSocketMapping * _pDeviceSocketMappingObj;

	std::vector<StreamSocket> _sockets;
};

#endif /* CSOCKETMANAGER_H_ */
