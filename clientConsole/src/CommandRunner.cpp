/*
 * CommandRunner.cpp
 *
 *  Created on: Dec 20, 2018
 *      Author: mikez
 */

#include "Poco/Net/NetException.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"

#include "CommandRunner.h"
#include "Logger.h"
#include "MsgPackager.h"

extern Logger * pLogger;

bool CommandRunner::Init(const Poco::Net::SocketAddress& deviceAddress)
{
	try
	{
		_socketAddress = deviceAddress;

		pLogger->LogInfo("CommandRunner::Init connecting to " + _socketAddress.toString());
		_socket.connect(_socketAddress);
		pLogger->LogInfo("CommandRunner::Init connected to " + _socketAddress.toString());

		_connected = true;
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandRunner::Init exception " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("CommandRunner::Init unknown exception");
	}

	return _connected;
}

bool CommandRunner::RunJsonCommand(const std::string& cmd)
{
	std::vector<unsigned char> pkg;

	//check connection
	if(!_connected) {
		pLogger->LogError("CommandRunner::SendCommand hasn't connected");
		return false;
	}

	//check JSON validity
	bool validJson = false;
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(cmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		validJson = true;
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandRunner::RunJsonCommand exception occurs: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("CommandRunner::RunJsonCommand unknown exception");
	}
	if(!validJson) {
		pLogger->LogError("CommandRunner::RunJsonCommand invalid json: " + cmd);
		return false;
	}

	//package JSON
	MsgPackager::PackageMsg(cmd, pkg);

	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	//move JSON to delivery stage
	for(unsigned int i=0; i<pkg.size(); i++) {
		_outgoing.push_back(pkg[i]);
	}

	return true;
}

void CommandRunner::onIncoming()
{
	std::vector<std::string> msgs;

	MsgPackager::RetrieveMsgs(_incoming, msgs);

	if(msgs.empty()) {
		return;
	}

	for(unsigned int i=0; i<msgs.size(); i++) {
		std::cout << "Reply received:\r\n";
		std::cout << msgs[i] << "\r\n";
	}
}

void CommandRunner::runTask()
{
	const int bufferLength = 1024;
	unsigned char buffer[bufferLength];
	Poco::Timespan timeSpan(10000); //10 ms
	Poco::Timespan zeroSpan;

	while(1)
	{
		if(isCancelled()) {
			_socket.close();
			break;
		}
		else
		{
			if(!_connected) {
				sleep(10);
				continue;
			}

			//send out _outgoing to device
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);

				if(_outgoing.size() > 0)
				{
					if(_socket.poll(zeroSpan, Poco::Net::Socket::SELECT_WRITE))
					{
						int amount = 0;

						for(auto it=_outgoing.begin(); it!=_outgoing.end(); it++) {
							buffer[amount] = *it;
							amount++;
							if(amount >= bufferLength) {
								break;
							}
						}
						amount = _socket.sendBytes(buffer, amount, 0);
						if(amount > 0) {
							pLogger->LogInfo("CommandRunner::runTask byte amount sent out: " + std::to_string(amount));
							//delete the data which was sent out.
							for(; amount>0; amount--) {
								_outgoing.pop_front();
							}
						}
						else {
							pLogger->LogError("CommandRunner::runTask failed sending: " + std::to_string(amount));
							_outgoing.clear();
						}
					}
				}
			}

			//read device's feedback
			if(_socket.poll(timeSpan, Poco::Net::Socket::SELECT_READ))
			{
				int amount = 0;
				bool errorOccur = false;

				try {
					amount = _socket.receiveBytes(buffer, bufferLength, 0);
					if(amount <= 0) {
						//peer socket has shut down
						errorOccur = true;
						pLogger->Log("CommandRunner::runTask peer socket shut down: " + std::to_string(amount));
					}
					else {
						//save content read to incoming queue
						for(int i=0; i<amount; i++) {
							_incoming.push_back(buffer[i]);
						}
						//process incoming data
						onIncoming();
					}
				}
				catch(Poco::TimeoutException& e) {
					; // timeout exception can be ignored.
				}
				catch(Poco::Net::NetException& e) {
					errorOccur = true;
					pLogger->LogError("CommandRunner::runTask exception " + e.displayText());
				}
				catch(...) {
					errorOccur = true;
					pLogger->LogError("CommandRunner::runTask unknown exception");
				}

				if(errorOccur)
				{
					pLogger->LogInfo("CommandRunner::runTask disconnect from " + _socketAddress.toString());
					_socket.close();
					_connected = false;
					_incoming.clear();
				}
			}
		}
	}

	pLogger->LogInfo("CommandRunner::runTask exited");
}
