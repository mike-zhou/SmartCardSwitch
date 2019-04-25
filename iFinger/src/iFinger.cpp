/*
 * iFinger.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: mikez
 */

#include <iostream>

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
#include "Poco/Path.h"
#include "Poco/File.h"

#include "Logger.h"
#include "CommandRunner.h"
#include "UserListener.h"
#include "WebServer.h"

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

class iFinger: public ServerApplication
{
public:
	iFinger(): _helpRequested(false)
	{
	}

	~iFinger()
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
				.callback(OptionCallback<iFinger>(this, &iFinger::handleHelp)));
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

		//use the designated configuration if it exist
		if(args.size() > 0)
		{
			std::string configFile = args[0];
			Poco::Path path(configFile);

			if(path.isFile())
			{
				Poco::File file(path);

				if(file.exists() && file.canRead()) {
					loadConfiguration(configFile);
				}
			}
		}

		TaskManager tmLogger;
		TaskManager tm;

		std::string logFolder;
		std::string logFile;
		std::string logFileSize;
		std::string logFileAmount;
		std::string coordinatePathFile;
		std::string movementConfigurationPathFile;

		//launch Logger
		try
		{
			//logs
			logFolder = config().getString("log_file_folder", "./logs/iFingerLogs");
			logFile = config().getString("log_file_name", "iFingerLog");
			logFileSize = config().getString("log_file_size", "1M");
			logFileAmount = config().getString("log_file_amount", "10");
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
		pLogger->CopyToConsole(true);
		tmLogger.start(pLogger); //tmLogger takes the ownership of pLogger.
		pLogger->LogInfo("**** iFinger V1.0.0 ****");

		// launch tasks
		try
		{
			CommandRunner * pCommandRunner;
			UserListener * pUserListener;
			WebServer * pWebServer;

			std::string proxyIp = config().getString("proxy_ip_address", "127.0.0.1");
			unsigned int proxyPort = config().getUInt("proxy_port");
			unsigned int lowClks = config().getUInt("lowClks");
			unsigned int highClks = config().getUInt("highClks");

			std::string userListenerIp = config().getString("user_listener_ip", "127.0.0.1");
			std::string userListenerPort = config().getString("user_listener_port", "60002");


			//CommandRunner
			pCommandRunner = new CommandRunner(lowClks, highClks, proxyIp, proxyPort);
			//user listener
			pUserListener = new UserListener(pCommandRunner);
			Poco::Net::SocketAddress userAddress(userListenerIp + ":" + userListenerPort);
			pUserListener->Bind(userAddress);


			//web server
			unsigned int webServerPort = config().getInt("web_server_port", 80);
			unsigned int webServerMaxQueue = config().getInt("web_server_max_queue", 128);
			unsigned int webServerMaxThreads = config().getInt("web_server_max_threads", 16);
			pWebServer = new WebServer(webServerPort, webServerMaxQueue, webServerMaxThreads, std::stoi(userListenerPort));

			//tm takes the ownership of tasks
			tm.start(pWebServer);
			tm.start(pUserListener);
			tm.start(pCommandRunner);
		}
		catch(Poco::Exception& e)
		{
			pLogger->LogError("main: exception in task starting :" + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("main: Unknown exception in task starting");
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


POCO_SERVER_MAIN(iFinger)

