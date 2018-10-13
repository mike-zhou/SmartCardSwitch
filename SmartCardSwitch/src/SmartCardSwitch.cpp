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
#include <iostream>
#include "CommandFactory.h"
#include "FeedbackParser.h"
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

	void sendCmdPkg(Poco::Net::StreamSocket& socket, std::vector<unsigned char>& cmdPkg)
	{
		unsigned int amount = 0;

		for(;;)
		{
			int count;

			count = cmdPkg.size() - amount;
			printf("sendCmdPkg: %d bytes to send\r\n", count);
			count = socket.sendBytes(cmdPkg.data() + amount, count, 0);
			if(count < 0) {
				printf("ERROR: sendCmdPkg error in socket.sendBytes\r\n");
			}
			printf("sendCmdPkg: %d bytes have been sent\r\n", count);
			amount += count;

			if(amount == cmdPkg.size()) {
				break;
			}
			if(amount > cmdPkg.size()) {
				printf("ERROR: sendCmdPkg sent more data out: %d:%d\r\n", amount, cmdPkg.size());
				break;
			}
		}
	}

	void receiveFeedbacks(Poco::Net::StreamSocket& socket, std::vector<std::string>& jsons)
	{
		std::deque<unsigned char> data;
		unsigned char buffer[1024];

		for(;;)
		{
			Poco::Timespan timeSpan(1000000);// 1 second

			if(socket.poll(timeSpan, Poco::Net::StreamSocket::SELECT_READ)) {
				int amount = socket.receiveBytes(buffer, 1024, 0);
				for(int i=0; i<amount; i++) {
					data.push_back(buffer[i]);
				}
				printf("receiveFeedbacks %d bytes of feedback arrives\r\n", amount);
				FeedbackParser::RetrieveFeedbacks(data, jsons);
				if(data.size() == 0) {
					break;
				}
				else {
					printf("receiveFeedbacks continue receiving data\r\n");
				}
			}
			else {
				printf("ERROR: receiveFeedbacks no data arrives in time constrain\r\n");
				break;
			}
		}
	}

	int main(const ArgVec& args)
	{
		if (_helpRequested) {
			return Application::EXIT_OK;
		}

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
		tm.start(pLogger);
		pLogger->LogInfo("**** SmartCardSwitch V1.0.0 ****");

		try
		{
			Poco::Net::SocketAddress socketAddress("127.0.0.1:60000");
			Poco::Net::StreamSocket socket;

			printf("Connecting to %s\r\n", socketAddress.toString().c_str());
			socket.connect(socketAddress);
			printf("Connected to %s\r\n", socketAddress.toString().c_str());

			{
				auto cmdVector = CommandFactory::DevicesGet();
				sendCmdPkg(socket, cmdVector);
			}
			{
				std::vector<std::string> jsons;
				receiveFeedbacks(socket, jsons);
				if(jsons.size() > 0) {
					for(auto it=jsons.begin(); it!=jsons.end(); it++)
					{
						printf("feedback: %s\r\n", it->c_str());
					}
				}
				else {
					printf("ERROR: no feedback in time constrain\r\n");
				}
			}

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

		waitForTerminationRequest();
		tm.cancelAll();
		tm.joinAll();
		delete pLogger;

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(SmartCardsSwitch)
