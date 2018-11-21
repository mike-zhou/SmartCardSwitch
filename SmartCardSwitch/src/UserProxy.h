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


class UserProxy: public Poco::Task, public IUserCommandRunnerObserver, public IUserPool
{
public:
	UserProxy();

	void SetUserCommandRunner(IUserCommandRunner * pRunner);

private:
	//Poco::Task
	void runTask();

	//IUserCommandRunnerObserver
	virtual void OnCommandStatus(const std::string& jsonStatus) override;

	//IUserPool
	virtual void AddSocket(StreamSocket& socket) override;

private:
	Poco::Mutex _mutex;

	std::vector<StreamSocket> _sockets;

	IUserCommandRunner * _pCmdRunner;

	std::deque<unsigned char> _input;
	std::deque<unsigned char> _output;

	std::string createErrorInfo(const std::string& info);
};

#endif /* USERPROXY_H_ */
