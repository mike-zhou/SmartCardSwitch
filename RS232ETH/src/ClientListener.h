/*
 * ClientListener.h
 *
 *  Created on: 9/09/2020
 *      Author: user1
 */

#ifndef CLIENTLISTENER_H_
#define CLIENTLISTENER_H_

#include "Poco/Task.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"

#include "SocketTransceiver.h"

class ClientListener: public Poco::Task
{
public:
	ClientListener(const std::string ip, unsigned int port);
	~ClientListener();
	void SetTransceiver(SocketTransceiver * pSocketTransceiver);
	void runTask() override;

private:
	SocketTransceiver * _pSocketTransceiver;
	std::string _ip;
	unsigned int _port;

	Poco::Net::ServerSocket _svrSocket;
};

#endif /* CLIENTLISTENER_H_ */
