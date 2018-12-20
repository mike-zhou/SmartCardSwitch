//============================================================================
// Name        : clientConsole.cpp
// Author      : Mike Zhou
// Version     :
// Copyright   : This software works with proxy to operate Advanced Smart Cards Switch. All of these belongs to Deeply Customized Device Ltd.
// Description : Hello World in C++, Ansi-style
//============================================================================

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

#include "ConsoleInput.h"
#include "CommandRunner.h"
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

class ClientConsole: public ServerApplication
{
public:
	ClientConsole(): _helpRequested(false)
	{
	}

	~ClientConsole()
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
				.callback(OptionCallback<ClientConsole>(this, &ClientConsole::handleHelp)));
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

		ConsoleInput * pConsoleInput;
		CommandRunner * pCmdRunner;
		Poco::Net::SocketAddress address("127.0.0.1:60001");
		std::string logPath = "./logs/clientConsole";
		std::string logFileName = "clientConsoleLog";
		std::string logFileSize = "1M";
		std::string logFileAmount = "10";

		pLogger = new Logger(logPath, logFileName, logFileSize, logFileAmount);
		pLogger->CopyToConsole(true);
		tmLogger.start(pLogger); //tmLogger takes the ownership of pLogger.
		pLogger->LogInfo("**** SmartCardSwitch Client ****");


		pCmdRunner = new CommandRunner;
		pConsoleInput = new ConsoleInput(pCmdRunner);

		if(pCmdRunner->Init(address))
		{
			tm.start(pCmdRunner);
			tm.start(pConsoleInput);

			waitForTerminationRequest();
			tm.cancelAll();
			tm.joinAll();
		}
		else
		{
			printf("failed to initialize CommandRunner\r\n");

			delete pCmdRunner;
			delete pConsoleInput;
		}

		tmLogger.cancelAll();
		tmLogger.joinAll();

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(ClientConsole)
