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
	void OnNewDevice(const std::string& deviceName);
	void OnDeviceReply(const long long socketId, const std::string& reply);
	void OnDeviceUnplugged(const long long socketId);

	void AddSocket(StreamSocket& socket);

	void runTask();

private:
	Poco::Mutex _mutex;

	const long long STARTING_SOCKET_ID = 1;
	long long _lastSocketId;
	std::map<long long, StreamSocket> _sockets;
	CDeviceSocketMapping * _pMapping;

	//send reply to socket
	bool sendReply(StreamSocket& socket, const std::string& reply);
	//process command from socket
	void onCommand(StreamSocket& socket, const std::string& command);
};

#endif /* CSOCKETMANAGER_H_ */
