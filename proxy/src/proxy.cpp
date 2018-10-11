#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Exception.h"
#include <iostream>

#include "CDeviceManager.h"
#include "CSocketManager.h"
#include "CListener.h"
#include "ProxyLogger.h"


using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Task;
using Poco::TaskManager;
using Poco::DateTimeFormatter;

ProxyLogger * pLogger;

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
		if (!_helpRequested)
		{
			TaskManager tm;
			Poco::Net::SocketAddress serverAddress;
			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;


			try
			{
				//client listener
				std::string ip = config().getString("listen_to_ip_address", "0.0.0.0");
				unsigned short port = config().getInt("listen_to_port", 60000);
				Poco::Net::IPAddress ipAddr(ip);
				serverAddress = Poco::Net::SocketAddress(ipAddr, port);
				//logs
				logFolder = config().getString("log_file_folder", "./logs/proxyLogs");
				logFile = config().getString("log_file_name", "proxyLog");
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

			pLogger = new ProxyLogger(logFolder, logFile, logFileSize, logFileAmount);
			tm.start(pLogger);

			CDeviceManager * pDeviceManager = new CDeviceManager;
			CSocketManager * pSocketManager = new CSocketManager;
			pDeviceManager->SetObserver(pSocketManager);
			pSocketManager->SetDevice(pDeviceManager);

			CListener * pListener = new CListener(pSocketManager);
			pListener->Bind(serverAddress);

			tm.start(pDeviceManager);
			tm.start(pSocketManager);
			tm.start(pListener);

			pDeviceManager->StartMonitoringDevices();

			waitForTerminationRequest();
			tm.cancelAll();
			tm.joinAll();
			delete pLogger;
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(Proxy)
