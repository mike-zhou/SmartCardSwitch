/*
 * WebServer.h
 *
 *  Created on: Jan 3, 2019
 *      Author: mikez
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <string>
#include <vector>
#include <deque>

#include "Poco/Task.h"
#include "Poco/Event.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"

class WebServer;

class KeyRequestHandler: public Poco::Net::HTTPRequestHandler
	/// Return a HTML document with the current date and time.
{
public:
	KeyRequestHandler(WebServer * pWebServer) { _pWebServer = pWebServer;}

private:
	//Poco::Net::HTTPRequestHandler
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

private:
	WebServer * _pWebServer;

	std::string getJsonCommand(Poco::Net::HTTPServerRequest& request);

	//***************************
	//command handlers
	//***************************
	//request:
	//	uri: /key
	//	body:
	//	 {
	//		"index":0
	//	 }
	void onKey(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
};


class KeyRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
public:
	KeyRequestHandlerFactory(WebServer * pWebServer) { _pWebServer = pWebServer; }

	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		return new KeyRequestHandler(_pWebServer);
	}

private:
	WebServer * _pWebServer;
};


class WebServer: public Poco::Task
{
public:
	WebServer(unsigned int port, unsigned int maxQueue, unsigned int maxThread, unsigned int userListenerPort);

	/**
	 * APIs for requests
	 */
	bool PressKey(unsigned int keyIndex, std::string & errorInfo);

private:
	//Poco::Task
	void runTask() override;

private:
	const unsigned long KEY_PRESSING_TIME_OUT = 3000000; //3 seconds

	Poco::Mutex _mutex;

	unsigned int _port;
	unsigned int _maxQueue;
	unsigned int _maxThread;

	Poco::Net::StreamSocket _iFingerSocket;
	unsigned int _userListenerPort;
	unsigned short _commandId;
	bool _bBusy;
};

#endif /* WEBSERVER_H_ */
