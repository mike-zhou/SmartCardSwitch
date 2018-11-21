/*
 * UserListener.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#include "Poco/Net/NetException.h"

#include "UserListener.h"
#include "Logger.h"

extern Logger * pLogger;

UserListener::UserListener(IUserPool * pPool):Task("UserListener")
{
	_pPool = pPool;

}

void UserListener::Bind(const SocketAddress& socketAddress)
{
	_svrAddress = socketAddress;
}

void UserListener::runTask()
{
	pLogger->LogInfo("UserListener bonds to " + _svrAddress.toString());
	_svrSocket.bind(_svrAddress);
	pLogger->LogInfo("UserListener starting listening ...");
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
				pLogger->LogInfo("UserListener::runTask new connection: " + clientAddr.toString());
				_pPool->AddSocket(socket);
			}
			catch(Poco::Net::NetException& e)
			{
				exceptionOccur = true;
				pLogger->LogError("UserListener::runTask exception: " + e.displayText());
			}
			catch(...)
			{
				exceptionOccur = true;
				pLogger->LogError("UserListener::runTask unknown exception occurs");
			}

			if(exceptionOccur) {
				sleep(5000);
			}
		}
	}

	pLogger->LogInfo("UserListener::runTask exited");
}



