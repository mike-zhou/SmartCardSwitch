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
#include "ISocketDeposit.h"

using Poco::Net::SocketAddress;

class CListener : public Poco::Task
{
public:
	CListener(ISocketDeposit * pSocketDeposit);
	virtual ~CListener();

	void Bind(const SocketAddress& socketAddress);

	void runTask();

private:
	ISocketDeposit * _pSocketDeposit;
};

#endif /* CLISTENER_H_ */
