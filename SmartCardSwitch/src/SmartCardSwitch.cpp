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
#include "CommandRunner.h"
#include "ConsoleOperator.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"
#include "UserProxy.h"
#include "UserListener.h"
#include "UserCommandRunner.h"

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
CoordinateStorage * pCoordinateStorage;
MovementConfiguration * pMovementConfiguration;

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
		std::string coordinatePathFile;
		std::string movementConfigurationPathFile;

		//launch Logger
		try
		{
			//logs
			logFolder = config().getString("log_file_folder", "./logs/SmartCardSwitchLogs");
			logFile = config().getString("log_file_name", "SmartCardSwitchLog");
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
		pLogger->LogInfo("**** SmartCardSwitch V1.0.0 ****");

		//static settings
		try
		{
			coordinatePathFile = config().getString("coordinate_storage_file", "./coordinate_storage");
			movementConfigurationPathFile = config().getString("movement_configuration_file", "./movement_configuration");
		}
		catch(Poco::Exception& e)
		{
			pLogger->LogError("main Config Exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("main Config unknown exception");
		}
		pCoordinateStorage = new CoordinateStorage(coordinatePathFile);
		pMovementConfiguration = new MovementConfiguration(movementConfigurationPathFile);

		// launch tasks
		try
		{
			DeviceAccessor * pDeviceAccessor;
			CommandRunner * pCommandRunner;
			ConsoleOperator * pConsoleOperator;
			UserListener * pUserListener;
			UserProxy * pUserProxy;
			UserCommandRunner * pUserCommandRunner;

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
			Poco::Net::SocketAddress userListenerAddress(userProxyListenerIp + ":" + userPorxyListenerPort);
			pUserProxy = new UserProxy(deviceToConnect, locatorNumberForReset, lineNumberForReset);
			pUserCommandRunner = new UserCommandRunner;
			pUserListener = new UserListener(pUserProxy);
			pUserListener->Bind(userListenerAddress);

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

			//tm takes the ownership of tasks
			tm.start(pCommandRunner);
			tm.start(pDeviceAccessor);
			tm.start(pConsoleOperator);
			tm.start(pUserCommandRunner);
			tm.start(pUserProxy);
			tm.start(pUserListener);
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


POCO_SERVER_MAIN(SmartCardsSwitch)
