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

#include "ISocketDeposit.h"

using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;

/**
 * This class listens for external socket connection request,
 * accepts connection requests,
 * and adds socket object to IUserPool instance.
 */
class UserListener: public Poco::Task
{
public:
	UserListener(ISocketDeposit * p);
	void Bind(const SocketAddress& socketAddress);

private:
	//Poco::Task
	void runTask();

private:
	ISocketDeposit * _pDeposit;

	SocketAddress _svrAddress;
	Poco::Net::ServerSocket _svrSocket;
};

#endif /* USERLISTENER_H_ */
