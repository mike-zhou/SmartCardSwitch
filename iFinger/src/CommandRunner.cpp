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
	_deviceCommandId = 0;

	_activeClientSocketIndex = -1;
}

void CommandRunner::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock lock(_mutex);

	if(_isPressing) {

	}
}

void CommandRunner::replyUser(StreamSocket & socket, const std::string & reply)
{
	std::vector<unsigned char> userReply;

	MsgPackager::PackageMsg(reply, userReply);
	try
	{
		socket.sendBytes(userReply.data(), userReply.size());
		pLogger->LogInfo("CommandRunner::replyUser sent user reply: " + reply);
	}
	catch(Poco::Exception &e) {
		pLogger->LogError("CommandRunner::replyUser exception in replying user: " + e.displayText());
	}
	catch(std::exception & e) {
		pLogger->LogError("CommandRunner::replyUser exception in replying user: " + std::string(e.what()));
	}
	catch(...) {
		pLogger->LogError("CommandRunner::replyUser unknown exception in replying user");
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
	const Poco::Timespan zeroSpan;

	std::string replyHeader;
	std::string replyHeaderFailure;
	std::string reply;

	//parse JSON command
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

	//verify user command
	if(command != USER_COMMAND)
	{
		reply = "{\"userCommand\":\"" + command + ",\"commandId\":\"" + cmdId + ",\"index\":" + std::to_string(index) + ",";
		reply = reply + "\"result\":\"failed\",\"errorInfo\":\"wrong command\"}";
	}
	else if(index >= SOLENOID_AMOUNT)
	{
		reply = "{\"userCommand\":\"" + command + ",\"commandId\":\"" + cmdId + ",\"index\":" + std::to_string(index) + ",";
		reply = reply + "\"result\":\"failed\",\"errorInfo\":\"index out of range\"}";
	}
	if(!reply.empty())
	{
		replyUser(socket, reply);
		return;
	}

	replyHeader = "{\"userCommand\":\"" + command + ",\"commandId\":\"" + cmdId + ",\"index\":" + std::to_string(index) + ",\"result\":\"";
	replyHeaderFailure = replyHeader + "failed" + "\",\"errorInfo\":\"";

	//check device's availability
	if(!_socketConnected)
	{
		reply = replyHeaderFailure + "proxy is not connected" + "\"}";
	}
	if(!_deviceConnected)
	{
		reply = replyHeaderFailure + "device is not connected" + "\"}";
	}
	if(_deviceSocket.poll(zeroSpan, Poco::Net::Socket::SELECT_ERROR))
	{
		//device socket has error
		reply = replyHeaderFailure + "proxy has to be disconnected" + "\"}";
		_deviceSocket.close();
		_socketConnected = false;
	}
	if(!_deviceSocket.poll(zeroSpan, Poco::Net::Socket::SELECT_WRITE))
	{
		//device socket is not ready for receiving data
		reply = replyHeaderFailure + "proxy cannot receive command" + "\"}";
	}
	if(!reply.empty())
	{
		replyUser(socket, reply);
		return;
	}

	//clear socket's receiving buffer
	for(;;)
	{
		unsigned char buffer[256];
		if(_deviceSocket.poll(zeroSpan, Poco::Net::Socket::SELECT_READ))
		{
			std::string garbage;
			char tmp[8];
			bool bException = false;

			try
			{
				int amount = _deviceSocket.receiveBytes(buffer, 256);

				for(int i=0; i<amount; i++)
				{
					sprintf(tmp, "%02x,", buffer[i]);
					garbage =  garbage + tmp;
				}
				pLogger->LogError("CommandRunner::onCommand garbge from device: " + garbage);
			}
			catch(Poco::Exception &e)
			{
				bException = true;
				pLogger->LogError("CommandRunner::onCommand exception in clearing socket buffer: " + e.displayText());
			}
			catch(std::exception & e)
			{
				bException = true;
				pLogger->LogError("CommandRunner::onCommand exception in clearing socket buffer: " + std::string(e.what()));
			}
			catch(...)
			{
				bException = true;
				pLogger->LogError("CommandRunner::onCommand unknown exception in clearing socket buffer");
			}

			if(bException)
			{
				_deviceSocket.close();
				_socketConnected = false;
				reply = replyHeaderFailure + "failure in clearing socket buffer" + "\"}";
				break;
			}
		}
		else
		{
			break;
		}
	}
	if(!reply.empty())
	{
		replyUser(socket, reply);
		return;
	}

	//send command to solenoid driver
	{
		std::string jsonCommand;
		std::vector<unsigned char> cmdPkg;
		int amount;

		_deviceCommandId++;

		jsonCommand += "{";
		jsonCommand += "\"command\":\"" + DEVICE_COMMAND + "\",";
		jsonCommand += "\"commandId\":" + std::to_string(_deviceCommandId) + ",";
		jsonCommand += "\"index\":" + std::to_string(index) + ",";
		jsonCommand += "\"lowClks\":" + std::to_string(_lowClks) + ",";
		jsonCommand += "\"highClks\":" + std::to_string(_highClks);
		jsonCommand += "}";

		MsgPackager::PackageMsg(jsonCommand, cmdPkg);
		amount = _deviceSocket.sendBytes(cmdPkg.data(), cmdPkg.size());
		if(amount != cmdPkg.size())
		{
			reply = replyHeaderFailure + "incomplete command is sent to proxy" + "\"}";
			replyUser(socket, reply);
			return;
		}
	}

	//receive reply from device
	{
		Poco::Timestamp receivingStart;
		Poco::Timespan timedSpan (10000); //10 milliseconds
		bool bTimeout = false;
		std::deque<unsigned char> deviceBytes;
		std::string commandReply;

		for(;;)
		{
			if(receivingStart.elapsed() > DEVICE_REPLY_TIMEOUT) {
				bTimeout = true;
				break;
			}

			if(_deviceSocket.poll(timedSpan, Poco::Net::Socket::SELECT_READ))
			{
				unsigned char buffer[1024];
				bool bException = false;

				try
				{
					int amount = _deviceSocket.receiveBytes(buffer, 1024);

					if(amount <= 0)
					{
						//peer socket has closed, or socket error
						_deviceSocket.close();
						_socketConnected = false;
						reply = replyHeaderFailure + "peer socket in proxy has been closed" + "\"}";
						pLogger->LogError("CommandRunner::onCommand error in receiving socket data: " + std::to_string(amount));
						break;
					}
					else if(amount > 0)
					{
						std::vector<std::string> cmdReplies;

						pLogger->LogInfo("CommandRunner::onCommand " + std::to_string(amount) + " bytes arrived");

						for(int i=0; i<amount; i++) {
							deviceBytes.push_back(buffer[i]);
						}

						MsgPackager::RetrieveMsgs(deviceBytes, cmdReplies);
						if(cmdReplies.empty()) {
							continue;
						}
						else
						{
							commandReply = cmdReplies[0]; // a reply is ready
							break;
						}
					}
				}
				catch(Poco::Exception &e)
				{
					bException = true;
					pLogger->LogError("CommandRunner::onCommand exception in receiving device reply: " + e.displayText());
				}
				catch(std::exception & e)
				{
					bException = true;
					pLogger->LogError("CommandRunner::onCommand exception in receiving device reply: " + std::string(e.what()));
				}
				catch(...)
				{
					bException = true;
					pLogger->LogError("CommandRunner::onCommand unknown exception in receiving device reply");
				}

				if(bException)
				{
					_deviceSocket.close();
					_socketConnected = false;
					reply = replyHeaderFailure + "exception occurred in receiving device reply" + "\"}";
					break;
				}
			}
		}
		if(bTimeout) {
			reply = replyHeaderFailure + "device didn't reply in time" + "\"}";
		}
		if(!reply.empty())
		{
			replyUser(socket, reply);
			return;
		}

		//parse device reply
		pLogger->LogInfo("CommandRunner::onCommand parsing device reply: " + commandReply);
		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var parsedReply = parser.parse(commandReply);
			Poco::JSON::Object::Ptr replyPtr = parsedReply.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *replyPtr;

			std::string command;
			unsigned int solenoidIndex;
			unsigned int lowClks;
			unsigned int highClks;
			unsigned int cmdId;
			std::string errorInfo;

			command = ds["command"].toString();
			solenoidIndex = ds["solenoidIndex"];
			lowClks = ds["lowClks"];
			highClks = ds["highClks"];
			cmdId = ds["commandId"];

			if(command != DEVICE_COMMAND) {
				reply = replyHeaderFailure + "wrong device reply" + "\"}";
				pLogger->LogError("CommandRunner::onCommand wrong device reply is returned");
			}
			else if(solenoidIndex != index) {
				reply = replyHeaderFailure + "wrong key index" + "\"}";
				pLogger->LogError("CommandRunner::onCommand wrong key index is returned");
			}
			else if(lowClks != _lowClks) {
				reply = replyHeaderFailure + "wrong data in device reply" + "\"}";
				pLogger->LogError("CommandRunner::onCommand wrong lowClks is returned");

			}
			else if(highClks != _highClks) {
				reply = replyHeaderFailure + "wrong data in device reply" + "\"}";
				pLogger->LogError("CommandRunner::onCommand wrong highClks is returned");

			}
			else if(replyPtr->has("error")) {
				errorInfo = ds["error"].toString();
				reply = replyHeaderFailure + errorInfo + "\"}";
			}
			else {
				reply = replyHeader + "succeeded" + "\"}";
			}
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
			reply = replyHeaderFailure + "exception occurred in parsing reply" + "\"}";
			pLogger->LogError("CommandRunner::onCommand exception occurred in parsing device reply: " + commandReply);
		}

		replyUser(socket, reply);
	}
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
						onCommand(*it, commands[0]); //run user command
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

				_deviceConnected = false;
			}
			else if(!_deviceConnected)
			{
				connectDevice();
			}
			else
			{
				pollClientSockets();
				pollDeviceSocket();
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
