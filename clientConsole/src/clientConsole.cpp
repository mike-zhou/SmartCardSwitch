//============================================================================
// Name        : clientConsole.cpp
// Author      : Mike Zhou
// Version     :
// Copyright   : This software works with proxy to operate Advanced Smart Cards Switch. All of these belongs to Deeply Customized Device Ltd.
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
#include <iostream>

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

class Proxy: public ServerApplication
{
public:
	Proxy(): _helpRequested(false)
	{
	}

	~Proxy()
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
				.callback(OptionCallback<Proxy>(this, &Proxy::handleHelp)));
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

		try
		{
			Poco::Net::SocketAddress socketAddress("127.0.0.1:60000");
			Poco::Net::StreamSocket socket;

			printf("Connecting to %s\r\n", socketAddress.toString().c_str());
			socket.connect(socketAddress);
			printf("Connected to %s\r\n", socketAddress.toString().c_str());


			printf("Socket is closing\r\n");
			socket.close();
			printf("Socket closed\r\n");
		}
		catch(Poco::Exception& e)
		{
			printf("Poco::Exception %s\r\n", e.displayText().c_str());
		}
		catch(...)
		{
			printf("Unknown exception occurs\r\n");
		}

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(Proxy)
