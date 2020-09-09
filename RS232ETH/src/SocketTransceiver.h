/*
 * SocketTransceiver.h
 *
 *  Created on: 9/09/2020
 *      Author: user1
 */

#ifndef SOCKETTRANSCEIVER_H_
#define SOCKETTRANSCEIVER_H_

#if defined(_WIN32) || defined(_WIN64)
#else

#include <string>
#include <deque>
#include <vector>

#include "Poco/Task.h"
#include "Poco/Net/StreamSocket.h"

#include "IDataExchange.h"
#include "IClientEvent.h"

class SocketTransceiver: public IDataExchange, public Poco::Task
{
public:
	SocketTransceiver(IClientEvent * pListener);
	~SocketTransceiver();

	bool SetSocket(Poco::Net::StreamSocket& socket);
	void Connect (IDataExchange * pInterface) override;
	void Send(const unsigned char * pData, const unsigned int amount) override;

private:
	void runTask() override;

private:
	IClientEvent * _pClientEventListener;
	IDataExchange * _pPeer;
	bool _socketValid;
	Poco::Net::StreamSocket _socket;

	Poco::Mutex _mutex;

	static const unsigned int MAX_BUFFER_SIZE = 0x10000;
	const unsigned int WRITING_TIMEOUT = 0; // 0 milliseconds
	const unsigned int READING_TIMEOUT = 10; // 10 milliseconds

	unsigned char _inputBuffer[MAX_BUFFER_SIZE];
	unsigned char _outputBuffer[MAX_BUFFER_SIZE];
	std::deque<unsigned char> _outputQueue; //data to peer socket

	void disconnectSocket();
};


#endif // defined(_WIN32) || defined(_WIN64)
#endif /* SOCKETTRANSCEIVER_H_ */
