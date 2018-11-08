/*
 * Logger.cpp
 *
 *  Created on: Sep 19, 2018
 *      Author: user1
 */

#include "../include/Logger.h"
#include "Poco/ScopedLock.h"
#include "stdio.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Path.h"
#include "Poco/File.h"

Logger::Logger(const std::string& folder,
		const std::string& name,
		const std::string& fileSize,
		const std::string& fileAmount):Task("Logger")
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
		std::string info = "Logger::Init exception occurs: " + e.displayText();

		printf("%s\r\n", info.c_str());
	}
	catch(...)
	{
		printf("Logger::Init unknown exception occurs");
	}
}

Logger::~Logger()
{

}

std::string Logger::currentTime()
{
	Poco::Timestamp currentTime;
	std::string rc = Poco::DateTimeFormatter::format(currentTime, Poco::DateTimeFormat::ISO8601_FRAC_FORMAT);

	return rc;
}

void Logger::Log(const std::string& log)
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

void Logger::LogError(const std::string& err)
{
	Log(err);
}

void Logger::LogDebug(const std::string& debug)
{
	Log(debug);
}

void Logger::LogInfo(const std::string& info)
{
	Log(info);
}

void Logger::CopyToConsole(bool copyToConsole)
{
	_copyToConsole = copyToConsole;
}

void Logger::runTask()
{
	while(1)
	{
		if(isCancelled())
		{
			for(; _logBuffer.size() > 0; )
			{
				std::string logLine = _logBuffer.front();
				_logBuffer.pop_front();
				if(_logFileInitialized) {
					_pLogger->trace(logLine);
				}
				else {
					printf("%s\r\n", logLine.c_str());
				}
			}

			if(_logFileInitialized) {
				_pLogger->trace(std::string("Logger::runTask exits"));
				_pLogger->close();
				_pLogger->shutdown();
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
						Log("Logger exception in log output: " + e.displayText());
					}
					catch(...)
					{
						_logFileInitialized = false; //output to console
						Log("Logger unknown exception in log output");
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
