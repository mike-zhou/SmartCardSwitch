/*
 * scsClient.cpp
 *
 *  Created on: Jan 22, 2019
 *      Author: mikez
 */
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"

#include "scsClientImp.h"
#include "CommandFactory.h"
#include "MsgPackager.h"

ScsClientImp * ScsClientImp::_pInstance = nullptr;
Poco::Mutex ScsClientImp::_mutex;

ScsClientImp::ScsClientImp(): ScsClient()
{
	_pLogger = nullptr;
	_portNumber = 0;
}

ScsClientImp::~ScsClientImp()
{
	_taskManager.cancelAll();
	_taskManager.joinAll();
	_pLogger = nullptr;
}

ScsClient * ScsClientImp::GetInstance()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_pInstance == nullptr) {
		_pInstance = new ScsClientImp();
	}

	return _pInstance;
}

ScsClient::ScsResult ScsClientImp::Initialize(const std::string& logFolder,
								const unsigned int fileSizeMB,
								const unsigned int fileAmount,
								const std::string & ipAddr,
								const unsigned int portNumber)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_pLogger != nullptr) {
		return ScsResult::InstanceWasInitialized;
	}

	_pLogger = new Logger(logFolder, "scsLog", std::to_string(fileSizeMB)+"M", std::to_string(fileAmount));
	_taskManager.start(_pLogger);

	_pLogger->LogInfo("ScsClientImp::Initialize succeeded");

	if(ipAddr.empty()) {
		_pLogger->LogError("ScsClientImp::Initialize empty IP address");
		return ScsResult::InvalidIpAddress;
	}
	_ipAddr = ipAddr;
	_portNumber = portNumber;

	return ScsResult::Succeess;
}

void ScsClientImp::Shutdown()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_pInstance != nullptr) {
		delete _pInstance;
	}
	_pInstance = nullptr;
}

std::string ScsClientImp::sendCommand(const std::string & command)
{
	Poco::Net::SocketAddress socketAddress(_ipAddr, _portNumber);

	Poco::Net::StreamSocket socket;
	std::vector<unsigned char> pkg;
	std::deque<unsigned char> replyDeque;
	const unsigned int bufferLength = 1024;
	unsigned char buffer[bufferLength];
	std::vector<std::string> msgs;

	_pLogger->LogInfo("ScsClientImp::sendCommand command: " + command);

	MsgPackager::PackageMsg(command, pkg);

	socket.connect(socketAddress);
	_pLogger->LogInfo("ScsClientImp::sendCommand connected to socket");

	for(unsigned int amount = 0; amount < pkg.size(); )
	{
		auto size = socket.sendBytes(pkg.data() + amount, pkg.size() - amount);
		_pLogger->LogInfo("ScsClientImp::sendCommand sent bytes: " + std::to_string(size));
		amount += size;
	}
	pkg.clear();

	_pLogger->LogInfo("ScsClientImp::sendCommand wait for reply");

	for(;;)
	{
		auto size = socket.receiveBytes(buffer, bufferLength);
		if(size <= 0) {
			//server close the socket
			throw Poco::Exception("ScsClientImp::sendCommand server closed socket");
		}
		else {
			_pLogger->LogInfo("ScsClientImp::sendCommand amount of bytes received: " + std::to_string(size));
			for(int i=0; i<size; i++) {
				replyDeque.push_back(buffer[i]);
			}
		}

		MsgPackager::RetrieveMsgs(replyDeque, msgs);
		if(msgs.size() > 0) {
			_pLogger->LogInfo("ScsClientImp::sendCommand amount of replies received: " + std::to_string(msgs.size()));
			break;
		}
	}
	socket.close();

	_pLogger->LogInfo("ScsClientImp::sendCommand reply received: " + msgs[0]);

	return msgs[0];
}

ScsClient::ScsResult ScsClientImp::InsertSmartCard(const unsigned int smartCardNumber)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::InsertSmartCard smart card number: " + std::to_string(smartCardNumber));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdInsertSmartCard(smartCardNumber);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::InsertSmartCard exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::InsertSmartCard unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::InsertSmartCard Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::InsertSmartCard Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::InsertSmartCard finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::RemoveSmartCard(const unsigned int smartCardNumber)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::RemoveSmartCard smart card number: " + std::to_string(smartCardNumber));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdRemoveSmartCard(smartCardNumber);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::RemoveSmartCard exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::RemoveSmartCard unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::RemoveSmartCard Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::RemoveSmartCard Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::RemoveSmartCard finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::SwipeSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::SwipeSmartCard smart card number: " + std::to_string(smartCardNumber) + ", " + std::to_string(pauseMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdSwipeSmartCard(smartCardNumber, pauseMs);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::SwipeSmartCard exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::SwipeSmartCard unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::SwipeSmartCard Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::SwipeSmartCard Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::SwipeSmartCard finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::TapSmartCard(const unsigned int smartCardNumber, const unsigned int pauseMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::TapSmartCard smart card number: " + std::to_string(smartCardNumber) + ", " + std::to_string(pauseMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdTapSmartCard(smartCardNumber, pauseMs);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::TapSmartCard exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::TapSmartCard unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::TapSmartCard Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::TapSmartCard Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::TapSmartCard finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::TapBarcode(const unsigned int smartCardNumber, const unsigned int pauseMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::TapBarcode smart card number: " + std::to_string(smartCardNumber) + ", " + std::to_string(pauseMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdShowBarCode(smartCardNumber, pauseMs);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::TapBarcode exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::TapBarcode unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::TapBarcode Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::TapBarcode Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::TapBarcode finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::PressPedKeys(const std::vector<unsigned int> pedNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::PressPedKeys key amount: " + std::to_string(pedNumbers.size()) + ", " + std::to_string(upPeriodMs) + ", " + std::to_string(downPeriodMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdPressPedKey(downPeriodMs, upPeriodMs, pedNumbers);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressPedKeys exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressPedKeys unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::PressPedKeys Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::PressPedKeys Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::PressPedKeys finished");

	return ScsResult::Succeess;
}


ScsClient::ScsResult ScsClientImp::PressSoftKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::PressSoftKeys key amount: " + std::to_string(keyNumbers.size()) + ", " + std::to_string(upPeriodMs) + ", " + std::to_string(downPeriodMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdPressSoftKey(downPeriodMs, upPeriodMs, keyNumbers);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressSoftKeys exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressSoftKeys unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::PressSoftKeys Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::PressSoftKeys Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::PressSoftKeys finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::PressAssistKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::PressAssistKeys key amount: " + std::to_string(keyNumbers.size()) + ", " + std::to_string(upPeriodMs) + ", " + std::to_string(downPeriodMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdPressAssistKey(downPeriodMs, upPeriodMs, keyNumbers);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressAssistKeys exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressAssistKeys unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::PressAssistKeys Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::PressAssistKeys Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::PressAssistKeys finished");

	return ScsResult::Succeess;
}

ScsClient::ScsResult ScsClientImp::PressTouchScreenKeys(const std::vector<unsigned int> keyNumbers, const unsigned int upPeriodMs, const unsigned int downPeriodMs)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_pLogger->LogInfo("ScsClientImp::PressTouchScreenKeys key amount: " + std::to_string(keyNumbers.size()) + ", " + std::to_string(upPeriodMs) + ", " + std::to_string(downPeriodMs));

	std::string command;
	std::string reply;
	std::string commandId;
	std::string replyId;
	std::string result;

	bool exceptionOccurred = false;

	command = CommandFactory::CmdTouchScreenKey(downPeriodMs, upPeriodMs, keyNumbers);
	try
	{
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			commandId = ds["commandId"].toString();
		}

		reply = sendCommand(command);

		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(reply);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			replyId = ds["commandId"].toString();
			result = ds["result"].toString();
		}

	}
	catch(Poco::Exception & e)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressTouchScreenKeys exception: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccurred = true;
		_pLogger->LogError("ScsClientImp::PressTouchScreenKeys unknown exception occurred");
	}

	if(exceptionOccurred) {
		return ScsResult::Failure;
	}
	else
	{
		if(replyId != commandId) {
			_pLogger->LogError("ScsClientImp::PressTouchScreenKeys Error: reply mismatch");
			return ScsResult::Failure;
		}
		else if(result != "succeeded") {
			_pLogger->LogError("ScsClientImp::PressTouchScreenKeys Error: failed to run command");
			return ScsResult::Failure;
		}
	}

	_pLogger->LogInfo("ScsClientImp::PressTouchScreenKeys finished");

	return ScsResult::Succeess;
}

ScsClient * GetScsClientInstance()
{
	return ScsClientImp::GetInstance();
}

