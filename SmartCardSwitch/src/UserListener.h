/*
 * UserListener.h
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#ifndef USERLISTENER_H_
#define USERLISTENER_H_

#include "Poco/Task.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"

using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;

class IUserPool
{
public:
	virtual ~IUserPool() { }

	virtual void AddSocket(StreamSocket& socket) = 0;
};

/**
 * This class listens for external socket connection request,
 * accepts connection requests,
 * and adds socket object to IUserPool instance.
 */
class UserListener: public Poco::Task
{
public:
	UserListener(IUserPool * p);
	void Bind(const SocketAddress& socketAddress);

private:
	//Poco::Task
	void runTask();

private:
	IUserPool * _pPool;

	SocketAddress _svrAddress;
	Poco::Net::ServerSocket _svrSocket;
};

#endif /* USERLISTENER_H_ */
