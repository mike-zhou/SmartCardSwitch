/*
 * IClientEvent.h
 *
 *  Created on: 9/09/2020
 *      Author: user1
 */

#ifndef ICLIENTEVENT_H_
#define ICLIENTEVENT_H_

class IClientEvent
{
public:
	virtual ~IClientEvent() {}
	virtual void OnClientSocketAddress(const std::string addrStr) = 0;
};


#endif /* ICLIENTEVENT_H_ */
