/*
 * UserProxy.cpp
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#include "Poco/ScopedLock.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"
#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"

#include "UserProxy.h"
#include "MsgPackager.h"
#include "Logger.h"

extern Logger * pLogger;

UserProxy::UserProxy(const std::string & deviceName, unsigned int locatorNumber, unsigned int lineNumber): Task("UserProxy")
{
	_deviceName = deviceName;
	_locatorNumberForReset = locatorNumber;
	_lineNumberForReset = lineNumber;

	_pUserCmdRunner = nullptr;
	_state = State::ConnectDevice;
}

void UserProxy::SetUserCommandRunner(IUserCommandRunner * pRunner)
{
	if(pRunner != nullptr) {
		_pUserCmdRunner = pRunner;
	}
}

void UserProxy::parseReply(const std::string& reply)
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(reply);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *objectPtr;

		std::string commandId;
		std::string state;
		std::string errorInfo;

		commandId = ds["commandId"].toString();
		state = ds["result"].toString();
		if(state == UserCmdStatusFailed) {
			errorInfo = ds["errorInfo"].toString();
		}
		else if(state == UserCmdStatusInternalError) {
			errorInfo = UserCmdStatusInternalError;
		}
		//todo: check all kinds of state before useing it in the following code.

		{
			//update user command result.
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);

			if(commandId == _commandId) {
				_commandState = state;
				_errorInfo = errorInfo;
			}
		}
	}
	catch(Poco::Exception &e)
	{
		pLogger->LogError("UserProxy::parseRely exception: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("UserProxy::parseRely unknown exception");
	}
}

void UserProxy::OnCommandStatus(const std::string& jsonStatus)
{
	pLogger->LogInfo("UserProxy::OnCommandStatus json reply: " + jsonStatus);

	switch(_state)
	{
		case State::ConnectDevice:
		case State::WaitForDeviceAvailability:
		case State::CheckResetKeyPressed:
		case State::WaitForResetPressed:
		case State::CheckResetKeyReleased:
		case State::WaitForResetReleased:
		case State::ResetDevice:
		case State::WaitForDeviceReady:
		{
			//replies for commands in initialization stage
			parseReply(jsonStatus);
		}
		break;

		case State::Normal:
		{
			//replies go to the other side of socket
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);
			std::vector<unsigned char> pkg;

			MsgPackager::PackageMsg(jsonStatus, pkg);
			pLogger->LogInfo("UserProxy::OnCommandStatus pkg size: " + std::to_string(pkg.size()));

			for(auto it=pkg.begin(); it!=pkg.end(); it++) {
				_output.push_back(*it);
			}
		}
		break;

		default:
		{
			pLogger->LogError("UserProxy::OnCommandStatus wrong state: " + std::to_string((int)_state));
		}
		break;
	}
}

std::string UserProxy::createErrorInfo(const std::string& info)
{
	return "{\"commandId\":\"invalid\", \"result\":\"failed\",\"errorInfo\":\"" + info + "\"}";
}

void UserProxy::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	switch(_state)
	{
		case State::ConnectDevice:
		case State::WaitForDeviceAvailability:
		{
			std::string errorInfo;
			std::vector<unsigned char> pkg;

			pLogger->LogInfo("UserProxy::AddSocket user proxy state: " + std::to_string((int)_state));
			pLogger->LogInfo("UserProxy::AddSocket device hasn't connected, refuse socket connection: " + socket.address().toString());

			errorInfo = createErrorInfo("no device is connected");

			MsgPackager::PackageMsg(errorInfo, pkg);

			socket.sendBytes(pkg.data(), pkg.size());
			socket.shutdownSend();
			socket.close();

			return;
		}
		break;

		case State::CheckResetKeyPressed:
		case State::WaitForResetPressed:
		{
			std::string errorInfo;
			std::vector<unsigned char> pkg;

			pLogger->LogInfo("UserProxy::AddSocket user proxy state: " + std::to_string((int)_state));
			pLogger->LogInfo("UserProxy::AddSocket waiting for reset confirm, refuse socket connection: " + socket.address().toString());

			errorInfo = createErrorInfo("reset confirm is needed");

			MsgPackager::PackageMsg(errorInfo, pkg);

			socket.sendBytes(pkg.data(), pkg.size());
			socket.shutdownSend();
			socket.close();

			return;
		}
		break;

		case State::ResetDevice:
		case State::WaitForDeviceReady:
		{
			std::string errorInfo;
			std::vector<unsigned char> pkg;

			pLogger->LogInfo("UserProxy::AddSocket user proxy state: " + std::to_string((int)_state));
			pLogger->LogInfo("UserProxy::AddSocket device hasn't been initialized, refuse socket connection: " + socket.address().toString());

			errorInfo = createErrorInfo("device hasn't been initialized");

			MsgPackager::PackageMsg(errorInfo, pkg);

			socket.sendBytes(pkg.data(), pkg.size());
			socket.shutdownSend();
			socket.close();

			return;
		}
		break;

		case State::Normal:
		{
			//device is ready to accept user command.
			//continue.
		}
		break;

		default:
		{
			std::string errorInfo;
			std::vector<unsigned char> pkg;

			pLogger->LogError("UserProxy::AddSocket user proxy state: " + std::to_string((int)_state));
			pLogger->LogError("UserProxy::AddSocket wrong device state, refuse socket connection: " + socket.address().toString());

			errorInfo = createErrorInfo("wrong device state");

			MsgPackager::PackageMsg(errorInfo, pkg);

			socket.sendBytes(pkg.data(), pkg.size());
			socket.shutdownSend();
			socket.close();

			return;
		}
		break;
	}

	if(_sockets.empty())
	{
		_sockets.push_back(socket);
		pLogger->LogInfo("UserProxy::AddSocket accepted socket connection: " + socket.address().toString());
	}
	else
	{
		std::string errorInfo;
		std::vector<unsigned char> pkg;
		auto ipaddress = _sockets[0].address();

		pLogger->LogInfo("UserProxy::AddSocket refused socket connection: " + socket.address().toString());

		errorInfo = createErrorInfo(ipaddress.toString() + " has already connected to this device");

		MsgPackager::PackageMsg(errorInfo, pkg);

		socket.sendBytes(pkg.data(), pkg.size());
		socket.shutdownSend();
		socket.close();
	}
}

bool UserProxy::sendDeviceConnectCommand()
{
	std::string error;
	std::string cmd = "{\"userCommand\":\"connect device\",\"deviceName\":\"" + _deviceName + "\",\"commandId\":\"connect device\"}";

	_commandId = "connect device";
	_commandState = UserCmdStatusOnGoing;

	_pUserCmdRunner->RunCommand(cmd, error);

	if(!error.empty())
	{
		pLogger->LogError("UserProxy::sendDeviceConnectCommand error: " + error);
		return false;
	}
	return true;
}

bool UserProxy::sendCheckResetPressedCommand()
{
	std::string error;
	std::string cmd = "{\"userCommand\":\"check reset pressed\",\"commandId\":\"check reset pressed\",\"locatorIndex\":" + std::to_string(_locatorNumberForReset) + ",\"lineNumber\":" + std::to_string(_lineNumberForReset) + "}";

	_commandId = "check reset pressed";
	_commandState = UserCmdStatusOnGoing;

	_pUserCmdRunner->RunCommand(cmd, error);

	if(!error.empty())
	{
		pLogger->LogError("UserProxy::sendCheckResetPressedCommand error: " + error);
		return false;
	}
	return true;
}

bool UserProxy::sendCheckResetReleasedCommand()
{
	std::string error;
	std::string cmd = "{\"userCommand\":\"check reset released\",\"commandId\":\"check reset released\",\"locatorIndex\":" + std::to_string(_locatorNumberForReset) + ",\"lineNumber\":" + std::to_string(_lineNumberForReset) + "}";

	_commandId = "check reset released";
	_commandState = UserCmdStatusOnGoing;

	_pUserCmdRunner->RunCommand(cmd, error);

	if(!error.empty())
	{
		pLogger->LogError("UserProxy::sendCheckResetReleasedCommand error: " + error);
		return false;
	}
	return true;
}

bool UserProxy::sendDeviceResetCommand()
{
	std::string error;
	std::string cmd = "{\"userCommand\":\"reset device\",\"commandId\":\"reset device\"}";

	_commandId = "reset device";
	_commandState = UserCmdStatusOnGoing;

	_pUserCmdRunner->RunCommand(cmd, error);

	if(!error.empty())
	{
		pLogger->LogError("UserProxy::sendDeviceConnectCommand error: " + error);
		return false;
	}
	return true;
}

void UserProxy::runTask()
{
	const unsigned int BufferSize = 1024;
	unsigned char buffer[BufferSize];
	Poco::Timespan timeSpan(10000); //10 ms
	Poco::Timestamp currentTime;
	Poco::Timestamp::TimeDiff deviceConnectInterval(DeviceConnectInterval);


	while(1)
	{
		if(isCancelled())
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex); //avoid conflicting with AddSocket

			//close socket
			if(!_sockets.empty())
			{
				std::string errorInfo;
				std::vector<unsigned char> pkg;
				auto ipaddress = _sockets[0].address();

				pLogger->LogInfo("UserProxy::runTask close socket connection: " + ipaddress.toString());

				errorInfo = createErrorInfo("SmartCardSwitch exists");

				MsgPackager::PackageMsg(errorInfo, pkg);

				_sockets[0].sendBytes(pkg.data(), pkg.size());
				_sockets[0].shutdownSend();
				_sockets[0].close();
				_sockets.clear();
			}

			break;
		}
		else
		{
			if(_state != State::Normal)
			{
				if(!currentTime.isElapsed(deviceConnectInterval)) {
					sleep(10);
					continue;
				}

				Poco::ScopedLock<Poco::Mutex> lock(_mutex);

				switch(_state)
				{
					case State::ConnectDevice:
					{
						_state = State::WaitForDeviceAvailability;

						if(!sendDeviceConnectCommand()) {
							_state = State::ConnectDevice;
							currentTime.update();
						}
					}
					break;

					case State::WaitForDeviceAvailability:
					{
						if(_commandState == UserCmdStatusOnGoing)
						{
							sleep(100); //wait for a short time for the reply.
						}
						if(_commandState == UserCmdStatusOnGoing)
						{
							currentTime.update(); //no reply has arrived yet, then wait for period of deviceConnectInterval
						}
						else if(_commandState == UserCmdStatusSucceeded)
						{
							_state = State::CheckResetKeyPressed;
						}
						else if(_commandState == UserCmdStatusFailed)
						{
							_state = State::ConnectDevice;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask failed to connect to device, error: " + _errorInfo);
						}
						else
						{
							_state = State::ConnectDevice;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask wrong command state: " + _commandState);
						}
					}
					break;

					case State::CheckResetKeyPressed:
					{
						_state = State::WaitForResetPressed;

						if(!sendCheckResetPressedCommand()) {
							_state = State::CheckResetKeyPressed;
							currentTime.update();
						}
					}
					break;

					case State::WaitForResetPressed:
					{
						if(_commandState == UserCmdStatusOnGoing)
						{
							sleep(100); //wait for a short time for the reply.
						}
						if(_commandState == UserCmdStatusOnGoing)
						{
							currentTime.update(); //no reply has arrived yet.
						}
						else if(_commandState == UserCmdStatusSucceeded)
						{
							_state = State::CheckResetKeyReleased;
						}
						else if(_commandState == UserCmdStatusFailed)
						{
							_state = State::CheckResetKeyPressed;
							pLogger->LogError("UserProxy::runTask failed to confirm reset, error: " + _errorInfo);
							currentTime.update();
						}
						else
						{
							_state = State::WaitForResetPressed;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask wrong command result: " + _commandState);
						}
					}
					break;

					case State::CheckResetKeyReleased:
					{
						_state = State::WaitForResetReleased;

						if(!sendCheckResetReleasedCommand()) {
							_state = State::CheckResetKeyPressed;
							currentTime.update();
						}
					}
					break;

					case State::WaitForResetReleased:
					{
						if(_commandState == UserCmdStatusOnGoing)
						{
							sleep(100); //wait for a short time for the reply.
						}
						if(_commandState == UserCmdStatusOnGoing)
						{
							currentTime.update(); //no reply has arrived yet.
						}
						else if(_commandState == UserCmdStatusSucceeded)
						{
							_state = State::ResetDevice;
						}
						else if(_commandState == UserCmdStatusFailed)
						{
							_state = State::CheckResetKeyReleased;
							pLogger->LogError("UserProxy::runTask failed to confirm reset, error: " + _errorInfo);
							currentTime.update();
						}
						else
						{
							_state = State::WaitForResetReleased;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask wrong command result: " + _commandState);
						}
					}
					break;

					case State::ResetDevice:
					{
						_state = State::WaitForDeviceReady;

						if(!sendDeviceResetCommand()) {
							_state = State::ResetDevice;
							currentTime.update();
						}
					}
					break;

					case State::WaitForDeviceReady:
					{
						if(_commandState == UserCmdStatusOnGoing)
						{
							sleep(100);
						}
						if(_commandState == UserCmdStatusOnGoing)
						{
							currentTime.update();
						}
						else if(_commandState == UserCmdStatusSucceeded)
						{
							_state = State::Normal;
						}
						else if(_commandState == UserCmdStatusFailed)
						{
							_state = State::ResetDevice;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask failed to reset device, error: " + _errorInfo);
						}
						else
						{
							_state = State::ConnectDevice;
							currentTime.update();
							pLogger->LogError("UserProxy::runTask wrong command result: " + _commandState);
						}
					}
					break;

					default:
					{
						pLogger->LogError("UserProxy::runTask wrong state: " + std::to_string((int)_state));
						_state = State::ConnectDevice;
						currentTime.update();
					}
					break;
				}

				continue;
			}

			if(_sockets.empty()) {
				sleep(10);
				continue;
			}

			bool exceptionOccured = false;
			bool peerClosed = false;

			try
			{
				//receive user command
				if(_sockets[0].poll(timeSpan, Poco::Net::Socket::SELECT_READ))
				{
					auto amount = _sockets[0].receiveBytes(buffer, BufferSize, 0);
					if(amount == 0) {
						peerClosed = true;
					}
					else
					{
						pLogger->LogInfo("UserProxy::runTask received bytes amount: " + std::to_string(amount));

						for(unsigned int i=0; i<amount; i++) {
							_input.push_back(buffer[i]);
						}

						std::vector<std::string> cmds;
						MsgPackager::RetrieveMsgs(_input, cmds);

						if(cmds.size() > 1) {
							pLogger->LogError("UserProxy::runTask multiple user commands arrived: " + std::to_string(cmds.size()));
						}
						//send user command to user command runner
						for(auto it=cmds.begin(); it!=cmds.end(); it++)
						{
							if(_pUserCmdRunner != nullptr)
							{
								std::string errorInfo;

								_pUserCmdRunner->RunCommand(*it, errorInfo);
								if(!errorInfo.empty())
								{
									std::vector<unsigned char> pkg;

									pLogger->LogError("UserProxy::runTask RunCommand error: " + errorInfo);
									errorInfo = createErrorInfo(errorInfo);

									Poco::ScopedLock<Poco::Mutex> lock(_mutex);

									MsgPackager::PackageMsg(errorInfo, pkg);
									pLogger->LogError("UserProxy::runTask error package size: " + std::to_string(pkg.size()));
									for(auto it=pkg.begin(); it!=pkg.end(); it++) {
										_output.push_back(*it);
									}
								}
							}
						}
					}
				}

				if(!peerClosed)
				{
					Poco::ScopedLock<Poco::Mutex> lock(_mutex);

					//send reply
					if(!_output.empty())
					{
						unsigned int dataSize;

						for(dataSize = 0; dataSize < _output.size(); dataSize++)
						{
							if(dataSize >= BufferSize) {
								break;
							}
							buffer[dataSize] = _output[dataSize];
						}

						if(dataSize > 0)
						{
							auto amount = _sockets[0].sendBytes(buffer, dataSize, 0);
							if(amount >= 0) {
								pLogger->LogInfo("UserProxy::runTask send out bytes amount: " + std::to_string(amount));
								//remove sent data from _output.
								for(; amount > 0; amount--) {
									_output.pop_front();
								}
							}
							else {
								pLogger->LogError("UserProxy::runTask error in sending out data: " + std::to_string(amount));
								peerClosed = true; // to close this socket.
							}
						}
					}
				}
			}
			catch(Poco::Exception& e)
			{
				pLogger->LogError("UserProxy::runTask exception occured: " + e.displayText());
				exceptionOccured = true;
			}
			catch(...)
			{
				pLogger->LogError("UserProxy::runTask unknown exception");
				exceptionOccured = true;
			}

			if(exceptionOccured || peerClosed) {
				pLogger->LogInfo("UserProxy::runTask close socket: " + _sockets[0].address().toString());
				_sockets[0].close();
				_sockets.clear();
			}
		}
	}

	pLogger->LogInfo("UserProxy::runTask exited");
}
