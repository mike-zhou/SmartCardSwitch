/*
 * CommandRunner.h
 *
 *  Created on: Dec 20, 2018
 *      Author: mikez
 */

#ifndef COMMANDRUNNER_H_
#define COMMANDRUNNER_H_

#include <memory>
#include <deque>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"

class CommandRunner: public Poco::Task
{
public:
	CommandRunner(): Task("ClientCommandRunner") { _connected = false;}
	virtual ~CommandRunner() {}

	bool Init(const Poco::Net::SocketAddress& deviceAddress);

	bool RunJsonCommand(const std::string& cmd);

private:
	void runTask();

private:
	Poco::Mutex _mutex;

	bool _connected;

	Poco::Net::SocketAddress _socketAddress;
	Poco::Net::StreamSocket _socket;
	std::deque<unsigned char> _incoming; //incoming data from socket
	std::deque<unsigned char> _outgoing; //outgoing data to socket

	void onIncoming();
};

#endif /* COMMANDRUNNER_H_ */
