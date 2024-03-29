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
#include "Poco/Path.h"
#include "Poco/File.h"

ProxyLogger::ProxyLogger(const std::string& folder,
		const std::string& name,
		const std::string& fileSize,
		const std::string& fileAmount):Task("ProxyLogger")
{
	_logFileInitialized = false;
	_overflowed = false;
	_copyToConsole = false;

	try
	{
		Poco::File logFolder(folder);
		logFolder.createDirectories();

		Poco::Path logFile(folder);
		logFile.makeDirectory();
		logFile.setFileName(name);

		//who deletes the allocated FileChannel object?
		Poco::FileChannel * pFileChannel = new Poco::FileChannel(logFile.toString());
		pFileChannel->setProperty("rotation", fileSize);
		pFileChannel->setProperty("archive", "timestamp");
		pFileChannel->setProperty("purgeCount", fileAmount);

		auto& logger = Poco::Logger::create("proxyLogger", pFileChannel, Poco::Message::PRIO_TRACE);
		_pLogger = &logger;
		_logFileInitialized = true;
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

	if(_copyToConsole) {
		printf("%s\r\n", logLine.c_str());
	}

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_overflowed) {
		if(_logBuffer.size() < (MAX_LINES - OVERFLOW_DIFF)) {
			_overflowed = false;
		}
	}
	if(!_overflowed) {
		_logBuffer.push_back(logLine);
		if(_logBuffer.size() >= MAX_LINES) {
			_overflowed = true;
			_logBuffer.push_back(currentTime() + " !!!! overflowed !!!!");
		}
	}
}

void ProxyLogger::LogError(const std::string& err)
{
	Log("ERROR: " + err);
}

void ProxyLogger::LogDebug(const std::string& debug)
{
	Log(debug);
}

void ProxyLogger::LogInfo(const std::string& info)
{
	Log(info);
}

void ProxyLogger::CopyToConsole(bool copyToConsole)
{
	_copyToConsole = copyToConsole;
}

void ProxyLogger::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			if(_logFileInitialized) {
				_pLogger->close();
			}
			break;
		}
		else
		{
			std::string logLine;
			{
				//retrieve a line of log
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);
				if(_logBuffer.size() > 0) {
					logLine = _logBuffer.front();
					_logBuffer.pop_front();
				}
			}
			if(logLine.size() > 0)
			{
				if(_logFileInitialized)
				{
					try
					{
						_pLogger->trace(logLine);
					}
					catch(Poco::Exception& e)
					{
						_logFileInitialized = false; //output to console
						Log("ProxyLogger exception in log output: " + e.displayText());
					}
					catch(...)
					{
						_logFileInitialized = false; //output to console
						Log("ProxyLogger unknown exception in log output");
					}
				}
				else
				{
					printf("%s\r\n", logLine.c_str());
				}
			}
			else {
				sleep(10);
			}
		}
	}
}
