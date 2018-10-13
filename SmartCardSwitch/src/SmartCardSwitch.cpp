//============================================================================
// Name        : SmartCardSwitch.cpp
// Author      : Mike Zhou
// Version     :
// Copyright   : This software works with proxy to operate Advanced Smart Cards Switch. All of these belongs to Deeply Customed Device Ltd.
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Exception.h"
#include "Poco/Format.h"
#include <iostream>
#include "CommandFactory.h"
#include "Logger.h"
#include "DeviceAccessor.h"

using namespace std;

using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Task;
using Poco::TaskManager;
using Poco::DateTimeFormatter;

Logger * pLogger;

class SmartCardsSwitch: public ServerApplication
{
public:
	SmartCardsSwitch(): _helpRequested(false)
	{
	}

	~SmartCardsSwitch()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
		logger().information("starting up");
	}

	void uninitialize()
	{
		logger().information("shutting down");
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<SmartCardsSwitch>(this, &SmartCardsSwitch::handleHelp)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample server application that demonstrates some of the features of the Util::ServerApplication class.");
		helpFormatter.format(std::cout);
	}

	int main(const ArgVec& args)
	{
		if (_helpRequested) {
			return Application::EXIT_OK;
		}

		TaskManager tmLogger;
		TaskManager tm;

		std::string logFolder;
		std::string logFile;
		std::string logFileSize;
		std::string logFileAmount;

		try
		{
			//logs
			logFolder = config().getString("log_file_folder", "./logs/SmartCardSwitchLogs");
			logFile = config().getString("log_file_name", "SmartCardSwitchLog");
			logFileSize = config().getString("log_file_size", "1M");
			logFileAmount = config().getString("log_file_amount", "10");
		}
		catch(Poco::NotFoundException& e)
		{
			logger().error("Config NotFoundException: " + e.displayText());
		}
		catch(Poco::SyntaxException& e)
		{
			logger().error("Config SyntaxException: " + e.displayText());
		}
		catch(Poco::Exception& e)
		{
			logger().error("Config Exception: " + e.displayText());
		}
		catch(...)
		{
			logger().error("Config unknown exception");
		}

		pLogger = new Logger(logFolder, logFile, logFileSize, logFileAmount);
		tmLogger.start(pLogger); //tmLogger takes the ownership of pLogger.
		pLogger->LogInfo("**** SmartCardSwitch V1.0.0 ****");

		try
		{
			DeviceAccessor * pDeviceAccessor;


			std::string proxyIp = config().getString("proxy_ip_address", "127.0.0.1");
			std::string proxyPort = config().getString("proxy_port", "60000");
			proxyIp = proxyIp + ":" + proxyPort;
			Poco::Net::SocketAddress socketAddress(proxyIp);
			pDeviceAccessor = new DeviceAccessor;
			pDeviceAccessor->Init(socketAddress);

			tm.start(pDeviceAccessor); //tm takes the ownership of pDeviceAccessor.
		}
		catch(Poco::Exception& e)
		{
			printf("Poco::Exception %s\r\n", e.displayText().c_str());
		}
		catch(...)
		{
			printf("Unknown exception occurs\r\n");
		}

		waitForTerminationRequest();

		//stop tasks
		tm.cancelAll();
		tm.joinAll();

		//stop logger task.
		tmLogger.cancelAll();
		tmLogger.joinAll();

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(SmartCardsSwitch)
