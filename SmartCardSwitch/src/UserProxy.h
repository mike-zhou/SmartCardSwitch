/*
 * UserProxy.h
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#ifndef USERPROXY_H_
#define USERPROXY_H_

#include <vector>
#include <deque>

#include "Poco/Task.h"
#include "Poco/Mutex.h"

#include "IUserCommandRunner.h"
#include "UserListener.h"


/**
 * This class accepts socket objects created by UserListener,
 * receives user commands from the socket,
 * passes those commands to UserCommandRunner instance,
 * and sends results to the other side of socket.
 */
class UserProxy: public Poco::Task, public IUserPool, public IUserCommandRunnerObserver
{
public:
	UserProxy();

	void SetUserCommandRunner(IUserCommandRunner * pRunner);
	void SetDeviceName(const std::string& deviceName);

private:
	//Poco::Task
	void runTask();

	//IUserCommandRunnerObserver
	virtual void OnCommandStatus(const std::string& jsonStatus) override;

	//IUserPool
	virtual void AddSocket(StreamSocket& socket) override;

private:
	const unsigned long DeviceConnectInterval = 10000000; //10 seconds

	Poco::Mutex _mutex;

	enum class State
	{
		ConnectDevice = 0,
		WaitForDeviceAvailability,
		AskForResetConfirm,
		WaitForResetConfirm,
		ResetDevice,
		WaitForDeviceReady,
		Normal
	};
	State _state;

	std::vector<StreamSocket> _sockets;

	IUserCommandRunner * _pUserCmdRunner;

	std::string _deviceName;
	std::string _commandId;
	std::string _commandState;
	std::string _errorInfo;

	void parseReply(const std::string& reply);
	bool sendDeviceConnectCommand();
	bool sendConfirmResetCommand();
	bool sendDeviceResetCommand();

	std::deque<unsigned char> _input;
	std::deque<unsigned char> _output;

	std::string createErrorInfo(const std::string& info);
};

#endif /* USERPROXY_H_ */
