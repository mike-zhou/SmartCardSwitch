/*
 * ProxyLogger.h
 *
 *  Created on: Sep 19, 2018
 *      Author: user1
 */

#ifndef PROXYLOGGER_H_
#define PROXYLOGGER_H_

#include <vector>
#include "Poco/Task.h"
#include "Poco/Path.h"
#include "Poco/Mutex.h"
#include "Poco/Logger.h"

class ProxyLogger: public Poco::Task {
public:
	ProxyLogger();
	virtual ~ProxyLogger();

	bool Init(const Poco::Path& path);
	void Log(const std::string& log);

	void runTask();

private:
	static const int MAX_LINES = 1024;

	Poco::Mutex _mutex;
	std::vector<std::string> _logBuffer;

	Poco::Logger * _pLogger;
};

#endif /* PROXYLOGGER_H_ */
