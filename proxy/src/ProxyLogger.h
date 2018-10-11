/*
 * ProxyLogger.h
 *
 *  Created on: Sep 19, 2018
 *      Author: user1
 */

#ifndef PROXYLOGGER_H_
#define PROXYLOGGER_H_

#include <deque>
#include <memory>
#include "Poco/Task.h"
#include "Poco/Path.h"
#include "Poco/Mutex.h"
#include "Poco/Logger.h"
#include "Poco/FileChannel.h"
#include "Poco/Channel.h"
#include "Poco/Message.h"

class ProxyLogger: public Poco::Task {
public:
	ProxyLogger(const std::string& folder, const std::string& name, const std::string& fileSize, const std::string& fileAmount);
	virtual ~ProxyLogger();

	void Log(const std::string& log);
	void LogError(const std::string& err);
	void LogDebug(const std::string& debug);
	void LogInfo(const std::string& info);

	void runTask();

private:
	static const int MAX_LINES = 10240;
	static const int OVERFLOW_DIFF = 1024;
	bool _overflowed;

	Poco::Mutex _mutex;
	std::deque<std::string> _logBuffer;

	Poco::Logger* _pLogger;
	bool _logFileInitialized;

	std::string currentTime();
};

#endif /* PROXYLOGGER_H_ */
