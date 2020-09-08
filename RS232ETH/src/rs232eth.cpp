/*
 * rs232eth.cpp
 *
 *  Created on: 8/09/2020
 *      Author: user1
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Semaphore.h"
#include "Poco/TaskManager.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"

#include "Logger.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::MessageHeader;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::CountingInputStream;
using Poco::NullOutputStream;
using Poco::StreamCopier;

std::string _serverURI;
std::string _clientIp;
Logger * pLogger;

class UserRequestHandler: public HTTPRequestHandler
{
public:
	UserRequestHandler()
	{
	}

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		const std::string & uri = request.getURI();
		if(uri != _serverURI) {
			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.send();
		}
		else
		{
			std::string reply;
			auto & oStream = response.send();

			response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
			reply = "{\"clientIp\":\"" + _clientIp + "\"}";
			oStream << reply;
		}
	}

private:
	std::string getJsonCommand(Poco::Net::HTTPServerRequest& request)
	{
		auto& iStream = request.stream();
		std::string json;

		//read body of request
		for(;;)
		{
			char c;

			iStream.read(&c, 1);
			if(iStream.eof()) {
				break;
			}
			else if(iStream.bad()) {
				pLogger->LogError("UserRequestHandler::getJsonCommand stream bad");
				break;
			}
			else if(iStream.fail()) {
				pLogger->LogError("UserRequestHandler::getJsonCommand stream fail");
				break;
			}

			json.push_back(c);
		}

		return json;
	}
};


class UserRequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	UserRequestHandlerFactory()
	{
	}

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		return new UserRequestHandler;
	}
};


class Rs232Eth: public Poco::Util::ServerApplication
{
public:
	Rs232Eth(): _helpRequested(false)
	{
	}

	~Rs232Eth()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A web server that shows how to work with HTML forms.");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			bool bClient;

			unsigned short httpServerPort;
			std::string httpServerIp;
			bool bException = false;
			HTTPServerParams * pServerParams;

			unsigned short comServerPort;
			std::string comServerIp;
			std::string comDevicePath;

			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;
			Poco::Net::SocketAddress socAddr;

			Poco::TaskManager tmLogger;

			//retrieve configuration
			try
			{
				bClient = (config().getInt("is_client") == 1);
				//http server
				httpServerPort = (unsigned short) config().getInt("server_port");
				httpServerIp = config().getString("server_ip");
				_serverURI = config().getString("server_uri");
				//com server
				comServerPort = (unsigned short) config().getInt("com_server_port");
				comServerIp = config().getString("com_server_ip");
				comDevicePath = config().getString("com_device_path");
				//log
				logFolder = config().getString("log_file_folder", "./logs/RS232ETH");
				logFile = config().getString("log_file_name", "rs232eth");
				logFileSize = config().getString("log_file_size", "1M");
				logFileAmount = config().getString("log_file_amount", "10");
			}
			catch(Poco::Exception & e)
			{
				std::cout << "Exception: " << e.displayText() << "\r\n";
				bException = true;
			}
			if(bException) {
				return Application::EXIT_CONFIG;
			}

			//start the logger
			pLogger = new Logger(logFolder, logFile, logFileSize, logFileAmount);
			pLogger->CopyToConsole(true);
			tmLogger.start(pLogger);
			pLogger->LogInfo("**** RS232ETH version 1.0.0 ****");

			pServerParams = new HTTPServerParams;
			pServerParams->setMaxThreads(30);
			pServerParams->setMaxQueued(64);

			// set-up a server socket
			socAddr = Poco::Net::SocketAddress (httpServerIp + ":" + std::to_string(httpServerPort));
			ServerSocket svs(socAddr);
			// set-up a HTTPServer instance
			HTTPServer srv(new UserRequestHandlerFactory, svs, pServerParams);
			// start the HTTPServer
			srv.start();
			pLogger->LogInfo("RS232ETH HTTP server is listening on: " + svs.address().toString());

			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the HTTPServer
			srv.stop();

			//stop logger
			tmLogger.cancelAll();
			tmLogger.joinAll();

		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


int main(int argc, char** argv)
{
	Rs232Eth app;
	return app.run(argc, argv);
}



