/*
 * iFinger.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: mikez
 */


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
#include <iostream>
#include "Logger.h"

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
			DeviceAccessor * pDeviceAccessor;
			CommandRunner * pCommandRunner;
			ConsoleOperator * pConsoleOperator;
			UserListener * pUserListener;
			UserProxy * pUserProxy;
			UserCommandRunner * pUserCommandRunner;
			WebServer * pWebServer;

			//device accessor
			std::string proxyIp = config().getString("proxy_ip_address", "127.0.0.1");
			std::string proxyPort = config().getString("proxy_port", "60000");
			proxyIp = proxyIp + ":" + proxyPort;
			Poco::Net::SocketAddress socketAddress(proxyIp);
			pDeviceAccessor = new DeviceAccessor;
			pDeviceAccessor->Init(socketAddress);

			//CommandRunner
			pCommandRunner = new CommandRunner;

			//ConsoleOperator
			pConsoleOperator = new ConsoleOperator(pCommandRunner);

			//user proxy
			std::string userProxyListenerIp = config().getString("user_proxy_listener_ip", "127.0.0.1");
			std::string userPorxyListenerPort = config().getString("user_proxy_listener_port", "60001");
			std::string deviceToConnect = config().getString("device_name", "Mixed_Motor_Drivers_HV1.0_SV1.0");
			unsigned int locatorNumberForReset = config().getInt("locator_number_for_reset", 2);
			unsigned int lineNumberForReset = config().getInt("line_number_for_reset", 8);
			bool autoBackToHome = config().getBool("auto_back_to_home_enabled", true);
			unsigned int autoBackToHomeSeconds = config().getUInt("auto_back_to_home_seconds", 300);
			Poco::Net::SocketAddress userListenerAddress(userProxyListenerIp + ":" + userPorxyListenerPort);
			pUserProxy = new UserProxy(deviceToConnect, locatorNumberForReset, lineNumberForReset, autoBackToHome, autoBackToHomeSeconds);
			pUserCommandRunner = new UserCommandRunner;
			pUserListener = new UserListener(pUserProxy);
			pUserListener->Bind(userListenerAddress);

			//web server
			unsigned int webServerPort = config().getInt("web_server_port", 80);
			unsigned int webServerMaxQueue = config().getInt("web_server_max_queue", 128);
			unsigned int webServerMaxThreads = config().getInt("web_server_max_threads", 16);
			std::string webServerFilesFolder = config().getString("web_server_folder", "wrongFolder");
			pWebServer = new WebServer(webServerPort, webServerMaxQueue, webServerMaxThreads, webServerFilesFolder);

			//couple tasks:
			// command flow: UserProxy >> UserCommandRunner >> ConsoleOperator >> CommandRunner >> DeviceAccessor
			// reply flow:   DeviceAccessor >> CommandRunner >> ConsoleOperator >> UsesrCommandRunner >> UserProxy
			pCommandRunner->SetDevice(pDeviceAccessor);
			pDeviceAccessor->AddObserver(pCommandRunner);
			pCommandRunner->AddResponseReceiver(pConsoleOperator);
			//couple user command runner with console operator
			pUserCommandRunner->SetConsoleOperator(pConsoleOperator);
			pConsoleOperator->AddObserver(pUserCommandRunner);
			//couple user command runner with user proxy
			pUserProxy->SetUserCommandRunner(pUserCommandRunner);
			pUserCommandRunner->AddObserver(pUserProxy);
			//couple web server with console operator
			pWebServer->SetConsoleOperator(pConsoleOperator);
			pConsoleOperator->AddObserver(pWebServer);

			//tm takes the ownership of tasks
			tm.start(pCommandRunner);
			tm.start(pDeviceAccessor);
			tm.start(pConsoleOperator);
			if(config().getBool("user_proxy_enable", true))
			{
				tm.start(pUserCommandRunner);
				tm.start(pUserProxy);
				tm.start(pUserListener);
			}
			else {
				pLogger->LogInfo("main user proxy is disabled");
			}
			if(config().getBool("web_server_enable", true)) {
				tm.start(pWebServer);
			}
			else {
				pLogger->LogInfo("web server is disabled");
			}
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

		delete pCoordinateStorage;
		delete pMovementConfiguration;

		//stop logger task.
		tmLogger.cancelAll();
		tmLogger.joinAll();

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(iFinger)

