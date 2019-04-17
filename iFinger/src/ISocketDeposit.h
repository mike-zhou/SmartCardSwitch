/*
 * ISocketDeposit.h
 *
 *  Created on: Oct 6, 2018
 *      Author: user1
 */

#ifndef ISOCKETDEPOSIT_H_
#define ISOCKETDEPOSIT_H_

#include "Poco/Net/StreamSocket.h"

using Poco::Net::StreamSocket;

class ISocketDeposit
{
public:
	virtual void AddSocket(StreamSocket& socket) = 0;
	virtual ~ISocketDeposit() {}
};



#endif /* ISOCKETDEPOSIT_H_ */
