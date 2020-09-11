/*
 * SocketTransceiver.cpp
 *
 *  Created on: 9/09/2020
 *      Author: user1
 */

#include "Poco/Net/NetException.h"

#include "SocketTransceiver.h"
#include "Logger.h"

extern Logger * pLogger;

SocketTransceiver::SocketTransceiver(IClientEvent * pListener) : Task("SocketTransceiver")
{
	_pClientEventListener = pListener;
	_pPeer = nullptr;
	_socketValid = false;
}

SocketTransceiver::~SocketTransceiver()
{

}

bool SocketTransceiver::SetSocket(Poco::Net::StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_socketValid) {
		pLogger->LogError("SocketTransceiver::SetSocket cannot accept socket: " + socket.address().toString());
		return false;
	}
	else
	{
		_outputQueue.clear();
		_socket = socket;

		if(_pClientEventListener != nullptr) {
			_pClientEventListener->OnClientSocketAddress(_socket.address().toString());
		}

		_socketValid = true;
		return true;
	}
}

void SocketTransceiver::Connect (IDataExchange * pInterface)
{
	if(pInterface == nullptr) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pPeer = pInterface;
}

void SocketTransceiver::Send(const unsigned char * pData, const unsigned int amount)
{
	if(pData == nullptr) {
		return;
	}
	if(amount == 0) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

    if (_socketValid)
    {
	    try
	    {
		    for(unsigned int i=0; i<amount; i++) {
			    _outputQueue.push_back(pData[i]);
		    }
	    }
	    catch(...)
	    {
		    pLogger->LogError("SocketTransceiver::Send exception happened in data saving");
	    }
    }
    else
    {
        pLogger->LogError("SocketTransceiver::Send discard byte amout: " + std::to_string(amount));
    }
}

void SocketTransceiver::runTask()
{
	Poco::Timespan readingSpan(READING_TIMEOUT * 1000);
	Poco::Timespan writingSpan(WRITING_TIMEOUT * 1000);

	while(1)
	{
		if(isCancelled()) {
			_socket.close();
			break;
		}
		else
		{
			if(!_socketValid){
				sleep(100);
				continue;
			}

			//send out _outgoing to device
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);

				if(_outputQueue.size() > 0)
				{
					if(_socket.poll(writingSpan, Poco::Net::Socket::SELECT_WRITE))
					{
						int amount = 0;

						for(auto it=_outputQueue.begin(); it!=_outputQueue.end(); it++) {
							_outputBuffer[amount] = *it;
							amount++;
							if(amount >= MAX_BUFFER_SIZE) {
								break;
							}
						}
						amount = _socket.sendBytes(_outputBuffer, amount, 0);
						if(amount > 0) {
							pLogger->LogInfo("SocketTransceiver::runTask sent out ETH bytes: " + std::to_string(amount));
							//delete the data which was sent out.
                            _outputQueue.erase(_outputQueue.begin(), _outputQueue.begin() + amount);
						}
						else {
							pLogger->LogError("SocketTransceiver::runTask failed in ETH sending, terminate socket connection");
							disconnectSocket();
						}
					}
				}
			}

			//read device's feedback
			if(_socket.poll(readingSpan, Poco::Net::Socket::SELECT_READ))
			{
				int amount = 0;
				bool errorOccur = false;

				try {
					amount = _socket.receiveBytes(_inputBuffer, MAX_BUFFER_SIZE, 0);
					if(amount <= 0) {
						//peer socket has shut down
						errorOccur = true;
						pLogger->Log("SocketTransceiver::runTask peer socket shut down: " + std::to_string(amount));
					}
					else
					{
                        pLogger->LogInfo("SocketTransceiver::runTask receive ETH bytes: " + std::to_string(amount));
						if(_pPeer != nullptr) {
							_pPeer->Send(_inputBuffer, amount);
						}
					}
				}
				catch(Poco::TimeoutException& e) {
					; // timeout exception can be ignored.
				}
				catch(Poco::Net::NetException& e) {
					errorOccur = true;
					pLogger->LogError("SocketTransceiver::runTask exception " + e.displayText());
				}
				catch(...) {
					errorOccur = true;
					pLogger->LogError("SocketTransceiver::runTask unknown exception");
				}

				if(errorOccur)
				{
					pLogger->LogInfo("SocketTransceiver::runTask error happened in reading, terminate socket connection");
					disconnectSocket();
				}
			}
		}
	}

	pLogger->LogInfo("SocketTransceiver::runTask exited");
}

void SocketTransceiver::disconnectSocket()
{
    Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_socket.close();
	_socketValid = false;

	if(_pClientEventListener != nullptr) {
		_pClientEventListener->OnClientSocketAddress("");
	}
}


