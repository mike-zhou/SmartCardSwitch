/*
 * ClientListener.cpp
 *
 *  Created on: 9/09/2020
 *      Author: user1
 */

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"

#include "ClientListener.h"
#include "Logger.h"

extern Logger * pLogger;

ClientListener::ClientListener(const std::string ip, unsigned int port): Task("ClientListener")
{
	_ip = ip;
	_port = port;
	_pSocketTransceiver = nullptr;

}

ClientListener::~ClientListener()
{

}

void ClientListener::SetTransceiver(SocketTransceiver * pSocketTransceiver)
{
	if(pSocketTransceiver == nullptr) {
		pLogger->LogError("ClientListener::SetTransceiver invalid parameter");
		return;
	}

	_pSocketTransceiver = pSocketTransceiver;
}


void ClientListener::runTask()
{
	Poco::Net::SocketAddress addr (_ip, _port);

	pLogger->LogInfo("UserListener bonds to " + addr.toString());
	_svrSocket.bind(addr);
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
				Poco::Net::SocketAddress clientAddr;

				auto socket = _svrSocket.acceptConnection(clientAddr);
				pLogger->LogInfo("ClientListener::runTask new connection: " + clientAddr.toString());
				if(_pSocketTransceiver != nullptr) {
					_pSocketTransceiver->SetSocket(socket);
				}
			}
			catch(Poco::Net::NetException& e)
			{
				exceptionOccur = true;
				pLogger->LogError("ClientListener::runTask exception: " + e.displayText());
			}
			catch(...)
			{
				exceptionOccur = true;
				pLogger->LogError("ClientListener::runTask unknown exception occurs");
			}

			if(exceptionOccur) {
				sleep(5000);
			}
		}
	}

	pLogger->LogInfo("ClientListener::runTask exited");
}









