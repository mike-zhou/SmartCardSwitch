/*
 * CommandRunner.h
 *
 *  Created on: Apr 17, 2019
 *      Author: mikez
 */

#ifndef COMMANDRUNNER_H_
#define COMMANDRUNNER_H_

#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

#include "ISocketDeposit.h"


/**
 * CommandRunner works as a server, which accepts socket connections from clients,
 * receives key press request, and sends the request to solenoid driver.
 * One key pressing command per socket at a time.
 *
 * command:
 * 	{
 * 		"userCommand":"press key",
 * 		"commandId":"unique command id",
 * 		"index":0
 * 	}
 *
 * reply:
 * 	{
 * 		"userCommand":"press key",
 * 		"commandId":"unique command id",
 * 		"index":0,
 * 		"result":"succeeded"|"failed",
 * 		"errorInfo":"error information"
 * 	}
 */
class CommandRunner: public Poco::Task, public ISocketDeposit
{
public:
	CommandRunner(unsigned long lowClks, unsigned long highClks, const std::string deviceIp, unsigned int devicePort);
	virtual ~CommandRunner() {}

protected:
	//ISocketDeposit
	virtual void AddSocket(StreamSocket& socket) override;

	//Poco::Task
	void runTask();

private:
	static const unsigned int SOLENOID_AMOUNT = 32;

	Poco::Mutex _mutex;
	unsigned long _lowClks;
	unsigned long _highClks;

	std::string _deviceIp;
	unsigned int _devicePort;

	std::vector<StreamSocket> _clientSockets; //sockets coming from clients
	int _activeClientSocketIndex;

	bool _socketConnected;
	bool _deviceConnected;
	bool _isPressing;
	StreamSocket _deviceSocket;

	std::vector<unsigned char> _command;
	std::vector<unsigned char> _reply;

	void onCommand(StreamSocket & socket, const std::string & cmd);
	void connectDevice();
	void pollClientSockets();
	void pollDeviceSocket();
};

#endif /* COMMANDRUNNER_H_ */
