/*
 * CommandRunner.cpp
 *
 *  Created on: Apr 17, 2019
 *      Author: mikez
 */

#include "Poco/Net/Socket.h"
#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"

#include "MsgPackager.h"
#include "CommandRunner.h"
#include "Logger.h"

extern Logger * pLogger;

CommandRunner::CommandRunner(unsigned long lowClks, unsigned long highClks, const std::string deviceIp, unsigned int devicePort) : Task("iFinger")
{
	_lowClks = lowClks;
	_highClks = highClks;
	_deviceIp = deviceIp;
	_devicePort = devicePort;

	_socketConnected = false;
	_deviceConnected = false;
	_isPressing = false;

	_activeClientSocketIndex = -1;
}

void CommandRunner::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock lock(_mutex);

	if(_isPressing) {

	}
}

void CommandRunner::connectDevice()
{

}

void CommandRunner::onCommand(StreamSocket & socket, const std::string & cmd)
{
	pLogger->LogInfo("CommandRunner::onCommand " + cmd);

	std::string command;
	std::string cmdId;
	unsigned int index;
	bool bException = false;

	std::vector<unsigned char> userReply;
	std::string reply;

	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var parsedCommand = parser.parse(cmd);
		Poco::JSON::Object::Ptr objectPtr = parsedCommand.extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *objectPtr;

		command = ds["userCommand"].toString();
		cmdId = ds["commandId"].toString();
		index = ds["index"];
	}
	catch(Poco::Exception &e)
	{
		bException = true;
		pLogger->LogError("CommandRunner::onCommand exception in parsing command: " + e.displayText());
	}
	catch(std::exception & e)
	{
		bException = true;
		pLogger->LogError("CommandRunner::onCommand exception in parsing command: " + std::string(e.what()));
	}
	catch(...)
	{
		bException = true;
		pLogger->LogError("CommandRunner::onCommand unknown exception");
	}

	if(bException)
	{
		pLogger->LogError("CommandRunner::onCommand ingore command");
		return;
	}

	//check command
	if(command != "press key")
	{
		reply = "{\"userCommand\":\"" + command + ",\"commandId\":\"" + cmdId + ",\"index\":" + std::to_string(index) + ",";
		reply = reply + "\"result\":\"failed\",\"errorInfo\":\"wrong command\"}";

		MsgPackager::PackageMsg(reply, userReply);
	}
	else if(index >= SOLENOID_AMOUNT)
	{
		reply = "{\"userCommand\":\"" + command + ",\"commandId\":\"" + cmdId + ",\"index\":" + std::to_string(index) + ",";
		reply = reply + "\"result\":\"failed\",\"errorInfo\":\"index out of range\"}";

		MsgPackager::PackageMsg(reply, userReply);
	}

	if(!userReply.empty())
	{
		try
		{
			socket.sendBytes(userReply.data(), userReply.size());
			pLogger->LogInfo("CommandRunner::onCommand sent user reply: " + reply);
		}
		catch(Poco::Exception &e) {
			pLogger->LogError("CommandRunner::onCommand exception in replying user: " + e.displayText());
		}
		catch(std::exception & e) {
			pLogger->LogError("CommandRunner::onCommand exception in replying user: " + std::string(e.what()));
		}
		catch(...) {
			pLogger->LogError("CommandRunner::onCommand unknown exception in replying user");
		}

		return;
	}

	//send command to solenoid driver

}

void CommandRunner::pollClientSockets()
{
	using Poco::Net::Socket;
	using Poco::Timespan;

	Socket::SocketList readList, writeList, exceptionList;
	Timespan timedSpan(10*1000); //10 ms
	Timespan zeroSpan;
	int socketAmount;

	//check if socket can accept replies/events.
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);

		for(int i = 0; i < _clientSockets.size(); i++)
		{
			readList.push_back(_clientSockets[i]);
			exceptionList.push_back(_clientSockets[i]);
		}
	}
	try
	{
		socketAmount = 0;
		socketAmount = Socket::select(readList, writeList, exceptionList, timedSpan);
	}
	catch(Poco::TimeoutException& e) {
		; //timeout, doesn't matter.
	}
	catch(Poco::Exception& e) {
		pLogger->LogError("CommandRunner::pollClientSockets exception in select: " + e.displayText());
	}
	catch(...) {
		pLogger->LogError("CommandRunner::pollClientSockets unknown exception in select");
	}

	if(socketAmount > 0)
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);

		//process only one socket
		for(auto it = _clientSockets.begin(); it != _clientSockets.end(); it++)
		{
			if(it->poll(zeroSpan, Socket::SELECT_ERROR))
			{
				pLogger->LogError("CommandRunner::pollClientSockets erase socket: " + it->address().toString());
				_clientSockets.erase(it);
				break;
			}
			else if(it->poll(zeroSpan, Socket::SELECT_READ))
			{
				//socket can be read
				unsigned char buf[1024];
				int amount = 0;
				bool bException = false;

				try
				{
					amount = it->receiveBytes(buf, 1024);
				}
				catch(Poco::Exception & e)
				{
					pLogger->LogError("CommandRunner::pollClientSockets exception in receiving: " + e.displayText());
					bException = true;
				}
				catch(...)
				{
					pLogger->LogError("CommandRunner::pollClientSockets unknown exception");
					bException = true;
				}

				if(bException)
				{
					pLogger->LogError("CommandRunner::pollClientSockets socket exception: " + it->address().toString());
					it->close();
					_clientSockets.erase(it);
					break;
				}
				if(amount == 0)
				{
					pLogger->LogInfo("CommandRunner::pollClientSockets peer socket closed: " + it->address().toString());
					it->close();
					_clientSockets.erase(it);
					break;
				}
				if(amount > 0)
				{
					pLogger->LogInfo("CommandRunner::pollClientSockets bytes amount: " + std::to_string(amount));

					std::deque<unsigned char> queue;
					std::vector<std::string> commands;

					for(int i=0; i<amount; i++)
					{
						queue.push_back(buf[i]);
					}

					MsgPackager::RetrieveMsgs(queue, commands);
					if(commands.empty())
					{
						pLogger->LogError("CommandRunner::pollClientSockets no valid command in :");

						std::string str;
						char buf[16];

						for(int i=0; i<amount; i++) {
							sprintf(buf, "%02x,", buf[i]);
							str = str + buf;
						}
						pLogger->LogError(str);
					}
					else {
						onCommand(*it, commands[0]);
					}

					break;
				}
			}
		}
	}
}

void CommandRunner::pollDeviceSocket()
{

}


void CommandRunner::runTask()
{
	pLogger->LogInfo("CommandRunner::runTask starts");

	while(1)
	{
		if(isCancelled()) {
			break;
		}

		try
		{
			if(!_socketConnected)
			{
				sleep(1000); //sleep 1 second

				Poco::Net::SocketAddress address(_deviceIp, _devicePort);
				Poco::Timespan span(1000000);// 1 second
				try
				{
					_deviceSocket.connect(address, span);
					_socketConnected = true;
				}
				catch(...)
				{
					pLogger->LogError("CommandRunner::runTask failed in connecting: " + address.toString());
				}
			}
			else
			{
				pollClientSockets();

				if(!_deviceConnected) {
					connectDevice();
				}
				else {
					pollDeviceSocket();
				}
			}
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError(("CommandRunner::runTask poco exception: " + e.displayText()));
		}
		catch(std::exception &e)
		{
			pLogger->LogError("CommandRunner::runTask std exception: " + std::string(e.what()));
		}
		catch(...)
		{
			pLogger->LogError("CommandRunner::runTask unknown exception");
		}
	}

	pLogger->LogInfo("CommandRunner::runTask exits");
}
