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
#include <deque>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Net/StreamSocket.h"
#include "IDeviceObserver.h"
#include "IDevice.h"


using Poco::Net::StreamSocket;

class CSocketManager : public Poco::Task, public IDeviceObserver {
public:
	CSocketManager();
	virtual ~CSocketManager();

	void SetDevice(IDevice * pDevice);

	//IDeviceObserver
	virtual void OnDeviceInserted(const std::string& deviceName) override;
	virtual void OnDeviceUnplugged(const std::string& deviceName) override;
	virtual void OnDeviceReply(const std::string& deviceName, const std::string& reply) override;


	void AddSocket(StreamSocket& socket);

	void runTask();

private:
	Poco::Mutex _mutex;

	const long long INVALID_SOCKET_ID = -1;
	const long long STARTING_SOCKET_ID = 1;
	long long _lastSocketId;

	//a map of socket id and socket object
	struct SocketWrapper
	{
		long long socketId;
		StreamSocket socket;
		std::deque<char> replyBuffer;
	};
	std::vector<struct SocketWrapper> _sockets;

	IDevice * _pDevice;

	//device has a 1:1 relationship to socket
	// a map of device name vs socket id
	std::map<std::string, long long> _deviceSocketMap;

	void onDeviceUnpluged(long long socketId);
	void onDeviceReply(long long socketId, const std::string& reply);

	//send reply to socket
	bool sendReply(StreamSocket& socket, const std::string& reply);
	//process command from socket
	void onCommand(StreamSocket& socket, const std::string& command);

	long long newSocketId() { return ++_lastSocketId; }
};

#endif /* CSOCKETMANAGER_H_ */
