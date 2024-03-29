#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include <iostream>

#include "CDeviceManager.h"
#include "CSocketManager.h"
#include "CListener.h"
#include "ProxyLogger.h"
#include "CDeviceMonitor.h"


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
			Poco::ThreadPool threadPool(2, 64);
			TaskManager tmLogger;
			TaskManager tm(threadPool);
			Poco::Net::SocketAddress serverAddress;
			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;
			std::vector<std::string> monitorFileVec;
			std::vector<std::string> controllingFileVec;
			std::vector<CDeviceMonitor *> monitorPointerVec;

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
				//controlling device file
				for(int i=0; ; i++)
				{
					char keyBuf[128];
					std::string controllingFile;

					sprintf(keyBuf, "controlling_device_file_%d", i);
					controllingFile = config().getString(keyBuf, std::string());

					if(controllingFile.empty()) {
						break;
					}
					else {
						controllingFileVec.push_back(controllingFile);
					}
				}
				//monitorFile
				for(int i=0; ; i++)
				{
					char keyBuf[128];
					std::string monitorFile;

					sprintf(keyBuf, "monitor_device_file__%d", i);
					monitorFile = config().getString(keyBuf, std::string());

					if(monitorFile.empty()) {
						break;
					}
					else {
						monitorFileVec.push_back(monitorFile);
					}
				}
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
			pLogger->CopyToConsole(true);
			tmLogger.start(pLogger);
			pLogger->LogInfo("**** proxy verion 1.0.0 ****");

			CDeviceManager * pDeviceManager = new CDeviceManager;
			CSocketManager * pSocketManager = new CSocketManager;
			if(controllingFileVec.empty()) {
				pLogger->LogError("no controlling file is specified");
			}
			for(unsigned int i=0; i < controllingFileVec.size(); i++)
			{
				pDeviceManager->AddDeviceFile(controllingFileVec[i]);
			}
			pDeviceManager->SetObserver(pSocketManager);
			pSocketManager->SetDevice(pDeviceManager);

			CListener * pListener = new CListener(pSocketManager);
			pListener->Bind(serverAddress);

			for(unsigned int i=0; i < monitorFileVec.size(); i++)
			{
				CDeviceMonitor * pMonitor = new CDeviceMonitor(monitorFileVec[i]);
				if(pMonitor == nullptr) {
					pLogger->LogError("failed to start monitor: " + monitorFileVec[i]);
				}
				else {
					monitorPointerVec.push_back(pMonitor);
				}
			}

			tm.start(pDeviceManager);
			tm.start(pSocketManager);
			tm.start(pListener);
			for(unsigned int i=0; i<monitorPointerVec.size(); i++) {
				tm.start(monitorPointerVec[i]);
			}

			waitForTerminationRequest();
			//stop tasks
			tm.cancelAll();
			tm.joinAll();
			//stop logger
			tmLogger.cancelAll();
			tmLogger.joinAll();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(Proxy)
