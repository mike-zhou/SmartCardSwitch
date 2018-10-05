/*
 * CListener.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CListener.h"

CListener::CListener(ISocketDeposit * pSocketDeposit):Task("Listener")
{
	_pSocketDeposit = pSocketDeposit;

}

CListener::~CListener()
{
	// TODO Auto-generated destructor stub
}

void CListener::Bind(const SocketAddress& socketAddress)
{

}

void CListener::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			sleep(1);
		}
	}
}
