/*
 * ProxyLogger.cpp
 *
 *  Created on: Sep 19, 2018
 *      Author: user1
 */

#include "ProxyLogger.h"
#include "Poco/ScopedLock.h"
#include "stdio.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"

ProxyLogger::ProxyLogger(const std::string& path, const std::string& fileSize, const std::string& fileAmount):Task("ProxyLogger")
{
	_initialized = false;
	try
	{
		Poco::FileChannel * pFileChannel = new Poco::FileChannel(path);
		pFileChannel->setProperty("rotation", fileSize);
		pFileChannel->setProperty("archive", "timestamp");
		pFileChannel->setProperty("purgeCount", fileAmount);

		auto& logger = Poco::Logger::create("proxyLogger", pFileChannel, Poco::Message::PRIO_TRACE);
		_pLogger = &logger;
		_initialized = true;
	}
	catch(Poco::Exception& e)
	{
		std::string info = "ProxyLogger::Init exception occurs: " + e.displayText();

		printf("%s\r\n", info.c_str());
	}
	catch(...)
	{
		printf("ProxyLogger::Init unknown exception occurs");
	}
}

ProxyLogger::~ProxyLogger()
{

}

std::string ProxyLogger::currentTime()
{
	Poco::Timestamp currentTime;
	std::string rc = Poco::DateTimeFormatter::format(currentTime, Poco::DateTimeFormat::ISO8601_FRAC_FORMAT);

	return rc;
}

void ProxyLogger::Log(const std::string& log)
{
	std::string logLine = currentTime() + " " + log;

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(!_initialized) {
		printf("%s\r\n", logLine.c_str());
		return;
	}
	else {
		_logBuffer.push_back(logLine);
	}
}

void ProxyLogger::LogError(const std::string& err)
{
	Log(err);
}

void ProxyLogger::LogDebug(const std::string& debug)
{
	Log(debug);
}

void ProxyLogger::LogInfo(const std::string& info)
{
	Log(info);
}

void ProxyLogger::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			if(_initialized) {
				_pLogger->close();
			}
			break;
		}
		else {
			if(!_initialized) {
				sleep(100);//sleep 100 ms
			}
			else {
				std::string logLine;
				{
					Poco::ScopedLock<Poco::Mutex> lock(_mutex);
					if(_logBuffer.size() > 0) {
						logLine = _logBuffer.front();
						_logBuffer.pop_front();
					}
				}
				if(logLine.size() > 0)
				{
					try
					{
						_pLogger->trace(logLine);
					}
					catch(Poco::Exception& e)
					{
						_initialized = false; //output to console
						Log("ProxyLogger exception in log output: " + e.displayText());
					}
					catch(...)
					{
						_initialized = false; //output to console
						Log("ProxyLogger unknown exception in log output");
					}
				}
				else {
					sleep(100);
				}
			}
		}
	}
}
