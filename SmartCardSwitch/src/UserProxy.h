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
	UserProxy(const std::string & deviceName, unsigned int locatorNumber, unsigned int lineNumber, bool autoBackToHome, unsigned int autoBackToHomeSeconds);

	void SetUserCommandRunner(IUserCommandRunner * pRunner);

private:
	//Poco::Task
	void runTask();

	//IUserCommandRunnerObserver
	virtual void OnCommandStatus(const std::string& jsonStatus) override;

	//IUserPool
	virtual void AddSocket(StreamSocket& socket) override;

private:
	const unsigned long DeviceConnectInterval = 1000000; //1 seconds

	const std::string ErrorDeviceNotConnected = "no device is connected";
	const std::string ErrorResetConfirmNeeded = "reset confirm is needed";
	const std::string ErrorDeviceNotInitialized = "device hasn't been initialized";
	const std::string ErrorWrongDeviceState = "wrong device state";

	Poco::Mutex _mutex;

	enum class State
	{
		ConnectDevice = 0,
		WaitForDeviceAvailability,
		CheckResetKeyPressed,
		WaitForResetPressed,
		CheckResetKeyReleased,
		WaitForResetReleased,
		ResetDevice,
		WaitForDeviceReady,
		Normal
	};
	State _state;

	std::vector<StreamSocket> _sockets;

	IUserCommandRunner * _pUserCmdRunner;

	unsigned int _locatorNumberForReset;
	unsigned int _lineNumberForReset;
	bool _autoBackToHomeEnabled;
	unsigned int _autoBackToHomeSeconds;
	std::string _deviceName;
	std::string _commandId;
	std::string _commandState;
	std::string _errorInfo;

	void parseReply(const std::string& reply);
	bool sendDeviceConnectCommand();
	bool sendCheckResetPressedCommand();
	bool sendCheckResetReleasedCommand();
	bool sendDeviceResetCommand();

	std::deque<unsigned char> _input;
	std::deque<unsigned char> _output;

	std::string createErrorInfo(const std::string& info, const std::string cmdId);
	std::string createErrorInfo(const std::string& info);
	std::string createAutoBackToHomeCmd();
};

#endif /* USERPROXY_H_ */
