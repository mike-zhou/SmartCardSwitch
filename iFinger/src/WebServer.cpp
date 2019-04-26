/*
 * WebServer.cpp
 *
 *  Created on: Jan 3, 2019
 *      Author: mikez
 */
#include "Poco/ThreadPool.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"

#include "WebServer.h"
#include "Logger.h"
#include "MsgPackager.h"

extern Logger * pLogger;

std::string KeyRequestHandler::getJsonCommand(Poco::Net::HTTPServerRequest& request)
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
			pLogger->LogError("KeyRequestHandler::getJsonCommand stream bad");
			break;
		}
		else if(iStream.fail()) {
			pLogger->LogError("KeyRequestHandler::getJsonCommand stream fail");
			break;
		}

		json.push_back(c);
	}

	return json;
}

void KeyRequestHandler::onKey(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("KeyRequestHandler::onKey no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("KeyRequestHandler::onKey command: " + command);
		unsigned int keyIndex;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			keyIndex = ds["index"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("KeyRequestHandler::onKey exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("KeyRequestHandler::onKey unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("KeyRequestHandler::onKey reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			if(!_pWebServer->PressKey(keyIndex, errorInfo)) {
				pLogger->LogError("KeyRequestHandler::onKey failed in key pressing: " + errorInfo);
			}

			if(errorInfo.empty())
			{
				pLogger->LogInfo("KeyRequestHandler::onKey key pressing succeeded");
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
			}
			else
			{
				pLogger->LogError("KeyRequestHandler::onKey key pressing failed");
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
			}
			response.send();
		}
	}

	pLogger->LogInfo("KeyRequestHandler::onKey request has been processed");
}


void KeyRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	const std::string& uri = request.getURI();
	pLogger->LogInfo("KeyRequestHandler::handleRequest %%%%%% URI: " + uri);

	if(uri == "/key") {
		onKey(request, response);
	}
	else
	{
		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("Bad request");
		response.send();
	}
}

/**
 *  WebServer
 */

WebServer::WebServer(unsigned int port, unsigned int maxQueue, unsigned int maxThread, unsigned int userListenerPort): Task("iFingerHttpServer")
{
	_port = port;
	_maxQueue = maxQueue;
	_maxThread = maxThread;
	_userListenerPort = userListenerPort;
	_bBusy = false;
	_commandId = 0;
}

bool WebServer::PressKey(unsigned int keyIndex, std::string & errorInfo)
{
	errorInfo.clear();

	//check if key is being pressed.
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);
		if(_bBusy) {
			errorInfo = "conflict: key is being pressed";
			return false;
		}
	}
	_bBusy = true;

	_commandId++;

	std::string jsonCmd;
	std::vector<unsigned char> cmdPkg;
	bool bException = false;
	std::string reply;
	std::deque<unsigned char> replyPkg;

	jsonCmd = "{";
	jsonCmd += "\"userCommand\":\"press key\",";
	jsonCmd += "\"commandId\":" + std::to_string(_commandId) + ",";
	jsonCmd += "\"index\":" + std::to_string(keyIndex);
	jsonCmd += "}";

	MsgPackager::PackageMsg(jsonCmd, cmdPkg);

	try
	{
		Poco::Timespan zeroSpan;
		Poco::Timespan timedSpan(1000000); // 1 second
		Poco::Net::SocketAddress address("127.0.0.1", _userListenerPort);
		unsigned int amount = 0;

		_iFingerSocket.connect(address, timedSpan);

		do
		{
			//send user command out
			for(; amount < cmdPkg.size();)
			{
				int size = _iFingerSocket.sendBytes(cmdPkg.data() + amount, cmdPkg.size() - amount);
				if(size < 1) {
					pLogger->LogInfo("WebServer::PressKey socket error in sending: " + std::to_string(size));
					errorInfo = "socket error in sending" + std::to_string(size);
					break;
				}
				else {
					amount += size;
					pLogger->LogInfo("WebServer::PressKey sent " + std::to_string(amount) + "/" + std::to_string(cmdPkg.size()) + " bytes to iFinger");
				}
			}
			if(!errorInfo.empty()) {
				break;
			}

			//receive reply
			Poco::Timestamp keyPressingTime;
			for(;;)
			{
				if(keyPressingTime.elapsed() > KEY_PRESSING_TIME_OUT) {
					errorInfo = "key pressing timed out";
					break;
				}

				if(_iFingerSocket.poll(zeroSpan, Poco::Net::Socket::SELECT_ERROR)) {
					pLogger->LogInfo("WebServer::PressKey socket error in receiving");
					errorInfo = "socket error in receiving";
					break;
				}
				if(_iFingerSocket.poll(timedSpan, Poco::Net::Socket::SELECT_READ))
				{
					std::vector<std::string> replies;
					unsigned char buffer[1024];
					int size = _iFingerSocket.receiveBytes(buffer, 1024);

					if(size < 1) {
						pLogger->LogInfo("WebServer::PressKey socket error in receiving: " + std::to_string(size));
						errorInfo = "socket error in receiving: " + std::to_string(size);
						break;
					}
					else
					{
						pLogger->LogInfo("WebServer::PressKey " + std::to_string(size) + " bytes arrived");
						for(int i=0; i<size; i++) {
							replyPkg.push_back(buffer[i]);
						}
					}

					MsgPackager::RetrieveMsgs(replyPkg, replies);
					pLogger->LogInfo("WebServer::PressKey " + std::to_string(replies.size()) + " replies arrived");
					if(!replies.empty()) {
						reply = replies[0];
						break;
					}
				}
			}
			if(!errorInfo.empty()) {
				break;
			}

			//parse reply
			pLogger->LogInfo("WebServer::PressKey parse reply: " + reply);
			{
				Poco::JSON::Parser parser;
				Poco::Dynamic::Var parsedReply = parser.parse(reply);
				Poco::JSON::Object::Ptr objectPtr = parsedReply.extract<Poco::JSON::Object::Ptr>();
				Poco::DynamicStruct ds = *objectPtr;


				std::string command;
				unsigned int commandId;
				unsigned int index;
				std::string result;

				command = ds["userCommand"].toString();
				if(command != "press key")
				{
					errorInfo = "wrong reply from device";
					pLogger->LogError("WebServer::PressKey " + errorInfo);
					break;
				}

				commandId = ds["commandId"];
				if(commandId != _commandId)
				{
					errorInfo = "wrong reply from device";
					pLogger->LogError("WebServer::PressKey " + errorInfo);
					break;
				}

				index = ds["index"];
				if(index != keyIndex)
				{
					errorInfo = "wrong reply from device";
					pLogger->LogError("WebServer::PressKey " + errorInfo);
					break;
				}

				result = ds["result"].toString();
				if(result != "succeeded")
				{
					errorInfo = ds["errorInfo"].toString();
					pLogger->LogError("WebServer::PressKey " + errorInfo);
					break;
				}

				pLogger->LogInfo("WebServer::PressKey key pressing finished");
			}
		}
		while(0);
	}
	catch(Poco::Exception &e)
	{
		bException = true;
		pLogger->LogError(("WebServer::PressKey poco exception: " + e.displayText()));
		errorInfo = e.displayText();
	}
	catch(std::exception &e)
	{
		bException = true;
		pLogger->LogError("WebServer::PressKey std exception: " + std::string(e.what()));
		errorInfo = e.what();
	}
	catch(...)
	{
		bException = true;
		pLogger->LogError("WebServer::PressKey unknown exception");
		errorInfo = "unknown exception";
	}

	_iFingerSocket.close();
	_bBusy = false;

	if(errorInfo.empty())
		return true;
	else
		return false;
}

void WebServer::runTask()
{
	Poco::ThreadPool::defaultPool().addCapacity(_maxThread);

	Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
	pParams->setMaxQueued(_maxQueue);
	pParams->setMaxThreads(_maxThread);

	pLogger->LogInfo("WebServer::runTask starts");

	try
	{
		// set-up a server socket
		Poco::Net::ServerSocket svs(_port);
		// set-up a HTTPServer instance
		Poco::Net::HTTPServer srv(new KeyRequestHandlerFactory(this), svs, pParams);
		// start the HTTPServer
		srv.start();
		// wait for CTRL-C or kill
		while(1)
		{
			if(isCancelled()) {
				break;
			}
			sleep(10);
		}
		// Stop the HTTPServer
		srv.stop();
	}
	catch(Poco::Exception &e)
	{
		pLogger->LogError("WebServer::runTask exception happened: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("WebServer::runTask unknown exception happened");
	}

	pLogger->LogInfo("WebServer::runTask exists");
}
