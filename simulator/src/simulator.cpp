//
// HTTPFormServer.cpp
//
// This sample demonstrates the HTTPServer and HTMLForm classes.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

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


#define BCM2708_PERI_BASE          0x3F000000 // for Pi 2 and Pi 3
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock
#define GPIO_PIN_AMOUNT 27

int  mem_fd;
void *gpio_map;
// I/O access
volatile unsigned *gpio;


static std::string _serverURI;
Logger * pLogger;

void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      pLogger->LogError("setup_io can't open /dev/mem");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      pLogger->LogError("setup_io mmap error");
      exit(-1);
   }

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;

   //set GPIO pins
   for(int i=0; i<GPIO_PIN_AMOUNT; i++) 
   {
	   // must use INP_GPIO before we can use OUT_GPIO
	   INP_GPIO(i);
	   OUT_GPIO(i);
   }

   pLogger->LogInfo("setup_io finished");

} // setup_io

bool setGpio(unsigned int index)
{
	if(index >= GPIO_PIN_AMOUNT) {
		pLogger->LogError("setGpioHigh wrong GPIO index: " + std::to_string(index));
		return false;
	}

	GPIO_SET = 1 << index;

	return true;
}

bool clearGpio(unsigned int index)
{
	if(index >= GPIO_PIN_AMOUNT) {
		pLogger->LogError("setGpioLow wrong GPIO index: " + std::to_string(index));
		return false;
	}

	GPIO_CLR = 1 << index;

	return true;
}

void wayneHandleRequest(unsigned int terminalIndex, unsigned int pumpIndex, unsigned int action)
{
	if(terminalIndex == 0) 
	{
		if(pumpIndex == 0) 
		{
			if(action == 0) {
				clearGpio(21);
			}
			else if(action == 1) {
				setGpio(21);
			}
			else {
				Poco::Exception("wayneHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else 
		{
			throw Poco::Exception("wayneHandleRequest wrong pump index: " + std::to_string(pumpIndex));
		}
	}
	else 
	{
		throw Poco::Exception("wayneHandleRequest wrong terminal index: " + std::to_string(terminalIndex));
	}
}

void gilbarcoHandleRequest(unsigned int terminalIndex, unsigned int item, unsigned int action)
{
	if(terminalIndex == 0) 
	{
		if(item == 0) 
		{
			if(action == 0) {
				clearGpio(12);
			}
			else if(action == 1) {
				setGpio(12);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 1) 
		{
			if(action == 0) {
				clearGpio(13);
			}
			else if(action == 1) {
				setGpio(13);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 2) 
		{
			if(action == 0) {
				clearGpio(19);
			}
			else if(action == 1) {
				setGpio(19);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 3) 
		{
			if(action == 0) {
				clearGpio(16);
			}
			else if(action == 1) {
				setGpio(16);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 4) 
		{
			if(action == 0) {
				clearGpio(26);
			}
			else if(action == 1) {
				setGpio(26);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 5) 
		{
			if(action == 0) {
				clearGpio(20);
			}
			else if(action == 1) {
				setGpio(20);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else if(item == 6) 
		{
			if(action == 0) {
				clearGpio(21);
			}
			else if(action == 1) {
				setGpio(21);
			}
			else {
				throw Poco::Exception("gilbarcoHandleRequest wrong action: " + std::to_string(action));
			}
		}
		else {
			throw Poco::Exception("gilbarcoHandleRequest wrong item: " + std::to_string(item));
		}
	}
	else 
	{
		throw Poco::Exception("gilbarcoHandleRequest wrong terminal index: " + std::to_string(terminalIndex));
	}
}

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
		}
		else
		{
			bool exceptionOccurred = true;
			std::string command = getJsonCommand(request);

			try
			{
				Poco::JSON::Parser parser;
				Poco::Dynamic::Var result = parser.parse(command);
				Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
				Poco::DynamicStruct ds = *objectPtr;
				std::string manufacture;

				manufacture = ds["manufacture"].toString();

				if(manufacture == "Wayne") 
				{
					unsigned int terminalIndex, pumpIndex, action;

					terminalIndex = ds["terminalIndex"];
					pumpIndex = ds["pumpIndex"];
					action = ds["action"];

					wayneHandleRequest(terminalIndex, pumpIndex, action);
				}
				else if(manufacture == "Gilbarco")
				{
					unsigned int terminalIndex, item, action;

					terminalIndex = ds["terminalIndex"];
					item = ds["item"];
					action = ds["action"];

					gilbarcoHandleRequest(terminalIndex, item, action);
				}
				else 
				{
					throw Poco::Exception("UserRequestHandler unknown manufacture: " + manufacture);
				}

				exceptionOccurred = false;
			}
			catch(Poco::Exception &e)
			{
				pLogger->LogError("UserRequestHandler exception: " + e.displayText());
			}
			catch(...)
			{
				pLogger->LogError("UserRequestHandler unknown exception occurred");
			}

			//reply to request
			if(exceptionOccurred)
			{
				pLogger->LogError("UserRequestHandler bad request");
				response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
				response.setReason("wrong parameter in: " + command);
			}
			else
			{
				pLogger->LogInfo("UserRequestHandler processed: " + command);
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
			}
		}
		response.send();
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


class SimulatorServer: public Poco::Util::ServerApplication
{
public:
	SimulatorServer(): _helpRequested(false)
	{
	}

	~SimulatorServer()
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
			unsigned short port;
			bool bException = false;
			HTTPServerParams * pServerParams;

			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;
			Poco::Net::SocketAddress socAddr ("0.0.0.0:8080");

			Poco::TaskManager tmLogger;

			//retrieve configuration
			try
			{
				port = (unsigned short) config().getInt("server_port");
				_serverURI = config().getString("server_uri");


				logFolder = config().getString("log_file_folder", "./logs/simulator");
				logFile = config().getString("log_file_name", "simulator");
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
			pLogger->LogInfo("**** simulator version 1.0.0 ****");

			setup_io();

			pServerParams = new HTTPServerParams;
			pServerParams->setMaxThreads(30);
			pServerParams->setMaxQueued(64);

			// set-up a server socket
			ServerSocket svs(socAddr);
			// set-up a HTTPServer instance
			HTTPServer srv(new UserRequestHandlerFactory, svs, pServerParams);
			// start the HTTPServer
			srv.start();
			pLogger->LogInfo("simulator is listening on: " + svs.address().toString());
			
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
	SimulatorServer app;
	return app.run(argc, argv);
}
