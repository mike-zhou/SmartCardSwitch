/*
 * CListener.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CLISTENER_H_
#define CLISTENER_H_

#include "Poco/Task.h"
#include "Poco/Net/SocketAddress.h"

#include "CSocketManager.h"

using Poco::Net::SocketAddress;

class CListener : public Poco::Task
{
public:
	CListener(CSocketManager * pSocketManager);
	virtual ~CListener();

	void Bind(const SocketAddress& socketAddress);

	void runTask();

private:
	CSocketManager * _pSocketManagerObj;
};

#endif /* CLISTENER_H_ */
