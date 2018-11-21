/*
 * UserProxy.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#include "Poco/ScopedLock.h"
#include "Poco/Timespan.h"

#include "UserProxy.h"
#include "MsgPackager.h"
#include "Logger.h"

extern Logger * pLogger;

UserProxy::UserProxy(): Task("UserProxy")
{
	_pCmdRunner = nullptr;
}

void UserProxy::SetUserCommandRunner(IUserCommandRunner * pRunner)
{
	if(pRunner != nullptr) {
		_pCmdRunner = pRunner;
	}
}

void UserProxy::OnCommandStatus(const std::string& jsonStatus)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	std::vector<unsigned char> pkg;

	pLogger->LogInfo("UserProxy::OnCommandStatus json reply: " + jsonStatus);

	MsgPackager::PackageMsg(jsonStatus, pkg);
	pLogger->LogInfo("UserProxy::OnCommandStatus pkg size: " + std::to_string(pkg.size()));

	for(auto it=pkg.begin(); it!=pkg.end(); it++) {
		_output.push_back(*it);
	}
}

std::string UserProxy::createErrorInfo(const std::string& info)
{
	return "{\"commandId\":\"invalid\", \"result\":\"failed\",\"errorInfo\":\"" + info + "\"}";
}

void UserProxy::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_sockets.empty())
	{
		_sockets.push_back(socket);
		pLogger->LogInfo("UserProxy::AddSocket accepted socket connection: " + socket.address().toString());
	}
	else
	{
		std::string errorInfo;
		std::vector<unsigned char> pkg;
		auto& ipaddress = _sockets[0].address();

		pLogger->LogInfo("UserProxy::AddSocket refused socket connection: " + socket.address().toString());

		errorInfo = createErrorInfo(ipaddress.toString() + " has already connected to this device");

		MsgPackager::PackageMsg(errorInfo, pkg);

		socket.sendBytes(pkg.data(), pkg.size());
		socket.shutdownSend();
		socket.close();
	}
}

void UserProxy::runTask()
{
	const unsigned int BufferSize = 1024;
	unsigned char buffer[BufferSize];
	Poco::Timespan timeSpan(10000); //10 ms


	while(1)
	{
		if(isCancelled())
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex); //avoid conflicting with AddSocket

			//close socket
			if(!_sockets.empty())
			{
				_sockets[0].close();
				_sockets.clear();
			}

			break;
		}
		else
		{
			if(_sockets.empty()) {
				sleep(10);
				continue;
			}

			bool exceptionOccured = false;
			bool peerClosed = false;

			try
			{
				//receive user command
				if(_sockets[0].poll(timeSpan, Poco::Net::Socket::SELECT_READ))
				{
					auto amount = _sockets[0].receiveBytes(buffer, BufferSize, 0);
					if(amount == 0) {
						peerClosed = true;
					}
					else
					{
						pLogger->LogInfo("UserProxy::runTask received bytes amount: " + std::to_string(amount));

						for(unsigned int i=0; i<amount; i++) {
							_input.push_back(buffer[i]);
						}

						std::vector<std::string> cmds;
						MsgPackager::RetrieveMsgs(_input, cmds);

						if(cmds.size() > 1) {
							pLogger->LogError("UserProxy::runTask multiple user commands arrived: " + std::to_string(cmds.size()));
						}
						//send user command to user command runner
						for(auto it=cmds.begin(); it!=cmds.end(); it++)
						{
							if(_pCmdRunner != nullptr)
							{
								std::string errorInfo;

								_pCmdRunner->Runcommand(*it, errorInfo);
								if(!errorInfo.empty())
								{
									std::vector<unsigned char> pkg;

									pLogger->LogError("UserProxy::runTask RunCommand error: " + errorInfo);
									errorInfo = createErrorInfo(errorInfo);

									MsgPackager::PackageMsg(errorInfo, pkg);
									pLogger->LogError("UserProxy::runTask error package size: " + std::to_string(pkg.size()));
									for(auto it=pkg.begin(); it!=pkg.end(); it++) {
										_output.push_back(*it);
									}
								}
							}
						}
					}
				}

				if(!peerClosed)
				{
					//send reply
					if(!_output.empty())
					{
						unsigned int dataSize;

						for(dataSize = 0; dataSize < _output.size(); dataSize++)
						{
							if(dataSize >= BufferSize) {
								break;
							}
							buffer[dataSize] = _output[dataSize];
						}

						if(dataSize > 0)
						{
							auto amount = _sockets[0].sendBytes(buffer, dataSize, 0);
							if(amount >= 0) {
								pLogger->LogInfo("UserProxy::runTask send out bytes amount: " + std::to_string(amount));
								//remove sent data from _output.
								for(; amount > 0; amount--) {
									_output.pop_front();
								}
							}
							else {
								pLogger->LogError("UserProxy::runTask error in sending out data: " + std::to_string(amount));
								peerClosed = true; // to close this socket.
							}
						}
					}
				}
			}
			catch(Poco::Exception& e)
			{
				pLogger->LogError("UserProxy::runTask exception occured: " + e.displayText());
				exceptionOccured = true;
			}
			catch(...)
			{
				pLogger->LogError("UserProxy::runTask unknown exception");
				exceptionOccured = true;
			}

			if(exceptionOccured || peerClosed) {
				pLogger->LogInfo("UserProxy::runTask close socket: " + _sockets[0].address().toString());
				_sockets[0].close();
				_sockets.clear();
			}
		}
	}

	pLogger->LogInfo("UserProxy::runTask exited");

}
