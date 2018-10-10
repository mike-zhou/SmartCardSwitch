/*
 * CListener.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CListener.h"
#include "ProxyLogger.h"
#include "Poco/Net/NetException.h"

extern ProxyLogger * pLogger;

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
	_svrAddress = socketAddress;
}

void CListener::runTask()
{
	pLogger->LogInfo("CListener bonds to " + _svrAddress.toString());
	_svrSocket.bind(_svrAddress);
	pLogger->LogInfo("CListener starting listening ...");
	_svrSocket.listen();

	while(1)
	{
		if(isCancelled()) {
			_svrSocket.close();
			break;
		}
		else
		{
			bool exceptionOccur = false;
			try
			{
				SocketAddress clientAddr;

				auto socket = _svrSocket.acceptConnection(clientAddr);
				pLogger->LogInfo("CListener::runTask new connection: " + clientAddr.toString());
				_pSocketDeposit->AddSocket(socket);
			}
			catch(Poco::Net::NetException& e)
			{
				exceptionOccur = true;
				pLogger->LogError("CListener::runTask exception: " + e.displayText());
			}
			catch(...)
			{
				exceptionOccur = true;
				pLogger->LogError("CListener::runTask unknown exception occurs");
			}

			if(exceptionOccur) {
				sleep(5000);
			}
		}
	}
}
