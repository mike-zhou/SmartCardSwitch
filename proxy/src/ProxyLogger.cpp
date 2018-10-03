/*
 * ProxyLogger.cpp
 *
 *  Created on: Sep 19, 2018
 *      Author: user1
 */

#include "ProxyLogger.h"
#include "Poco/ScopedLock.h"
#include "stdio.h"

ProxyLogger::ProxyLogger():Task("ProxyLogger")
{
	_pLogger = NULL;

}

ProxyLogger::~ProxyLogger() {
}

bool ProxyLogger::Init(const Poco::Path& path)
{
	return true;
}

void ProxyLogger::Log(const std::string& log)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	printf("%s\r\n", log.c_str());
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
			break;
		}
		else {
			sleep(100);//sleep 100 ms
		}
	}
}
