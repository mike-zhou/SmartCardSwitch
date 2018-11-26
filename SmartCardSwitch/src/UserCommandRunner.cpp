/*
 * UserCommandRunner.cpp
 *
 *  Created on: Nov 20, 2018
 *      Author: mikez
 */

#include "Poco/Exception.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"

#include "UserCommandRunner.h"
#include "Logger.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

extern Logger * pLogger;
extern CoordinateStorage * pCoordinateStorage;
extern MovementConfiguration * pMovementConfiguration;

UserCommandRunner::UserCommandRunner() : Task("UserCommandRunner")
{
	_smartCardSlotWithCard = false;
	_deviceHomePositioned = false;
	_clampState = ClampState::Released;
	_currentPosition = CoordinateStorage::Type::Home;
	_userCommand.state = CommandState::Idle;
	_consoleCommand.state = CommandState::Idle;
	_pConsoleOperator = nullptr;
}

void UserCommandRunner::notifyObservers(const std::string& cmdId, CommandState state, const std::string& errorInfo)
{
	std::string reply;
	std::string strState;

	reply = "{";
	reply = reply + "\"commandId\":\"" + cmdId + "\",";

	switch(state)
	{
		case CommandState::OnGoing:
		{
			strState = "ongoing";
		}
		break;

		case CommandState::Failed:
		{
			strState = "failed";
		}
		break;

		case CommandState::Succeeded:
		{
			strState = "succeed";
		}
		break;

		default:
		{
			strState = "internal error";
		}
		break;
	}
	reply = reply + "\"result\":\"" + strState + "\"";

	if(state == CommandState::Failed) {
		reply = reply + ",\"errorInfo\":\"" + errorInfo + "\"";
	}

	reply = reply + "}";

	pLogger->LogInfo("UserCommandRunner::notifyObservers reply: " + reply);

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnCommandStatus(reply);
	}
}

void UserCommandRunner::parseUserCmdConnectDevice(Poco::DynamicStruct& ds)
{
	std::string deviceName = ds["deviceName"].toString();

	if(deviceName.empty()) {
		std::string err = "UserCommandRunner::parseUserCmdConnectDevice empty device name";
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.deviceName = deviceName;
}

bool UserCommandRunner::expandUserCmdConnectDevice()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdDevicesGet();
	_userCommand.consoleCommands.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdDeviceConnect(0);
	_userCommand.consoleCommands.push_back(cmd);

	return true;
}

void UserCommandRunner::parseUserCmdResetDevice(Poco::DynamicStruct& ds)
{
	//nothing further to be done
}


void UserCommandRunner::parseUserCmdSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		std::string err = "UserCommandRunner::parseUserCmdSmartCard smart card number of range: " + std::to_string(number);
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.smartCardNumber = number;
}

void UserCommandRunner::parseUserCmdBarCode(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->BarCodeCardsAmout())
	{
		std::string err = "UserCommandRunner::parseUserCmdShowBarCode bar code card number of range: " + std::to_string(number);
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.smartCardNumber = number;
}

void UserCommandRunner::parseUserCmdKeys(Poco::DynamicStruct& ds)
{
	_userCommand.downPeriod = ds["downPeriod"];
	_userCommand.upPeriod = ds["upPeriod"];
	_userCommand.keyNumbers.clear();

	auto keyAmount = ds["keys"].size();

	for(unsigned int i=0; i<keyAmount; i++) {
		_userCommand.keyNumbers.push_back(0);
	}

	for(unsigned int i=0; i<keyAmount; i++)
	{
		unsigned int index = ds["keys"][i]["index"];
		unsigned int number = ds["keys"][i]["keyNumber"];

		if(index < keyAmount) {
			if(number < pCoordinateStorage->PedKeysAmount()) {
				_userCommand.keyNumbers[index] = number;
			}
			else {
				std::string err = "UserCommandRunner::parseUserCmdPressPedKey key number of range: " + std::to_string(number);
				pLogger->LogError(err);
				throw Poco::Exception(err);
			}
		}
		else {
			std::string err = "UserCommandRunner::parseUserCmdPressPedKey index out of range: " + std::to_string(index);
			pLogger->LogError(err);
			throw Poco::Exception(err);
		}
	}
}

bool UserCommandRunner::expandUserCmdResetDevice()
{

}

bool UserCommandRunner::expandUserCmdInsertSmartCard()
{

}

bool UserCommandRunner::expandUserCmdRemoveSmartCard()
{

}

bool UserCommandRunner::expandUserCmdSwipeSmartCard()
{

}

bool UserCommandRunner::expandUserCmdTapSmartCard()
{

}

bool UserCommandRunner::expandUserCmdShowBarCode()
{

}

bool UserCommandRunner::expandUserCmdPressPedKey()
{

}

bool UserCommandRunner::expandUserCmdPressSoftKey()
{

}

bool UserCommandRunner::expandUserCmdPressAssistKey()
{

}

bool UserCommandRunner::expandUserCmdTouchScreen()
{

}

void UserCommandRunner::RunCommand(const std::string& jsonCmd, std::string& errorInfo)
{
	Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex);

	if(_deviceHomePositioned == false) {
		errorInfo = ErrorDeviceNotHomePositioned;
		pLogger->LogError("UserCommandRunner::RunCommand device not home positioned, denied: " + jsonCmd);
		return;
	}

	if(_userCommand.state != CommandState::Idle) {
		errorInfo = ErrorUserCommandOnGoing;
		pLogger->LogError("UserCommandRunner::RunCommand command ongoing, denied: " + jsonCmd);
		return;
	}

	//parse user command
	bool cmdParseError = true;
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *objectPtr;

		_userCommand.command = ds["userCommand"].toString();
		_userCommand.commandId = ds["userCommandId"].toString();

		if(_userCommand.command == UserCmdInsertSmartCard) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdRemoveSmartCard) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdSwipeSmartCard) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdTapSmartCard) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdShowBarCode) {
			parseUserCmdBarCode(ds);
		}
		else if(_userCommand.command == UserCmdPressPedKey) {
			parseUserCmdKeys(ds);
		}
		else if(_userCommand.command == UserCmdPressSoftKey) {
			parseUserCmdKeys(ds);
		}
		else if(_userCommand.command == UserCmdPressAssistKey) {
			parseUserCmdKeys(ds);
		}
		else if(_userCommand.command == UserCmdTouchScreen) {
			parseUserCmdKeys(ds);
		}
		else {
			errorInfo = ErrorUnSupportedCommand;
			pLogger->LogError("UserCommandRunner::RunCommand unsupported command, denied: " + jsonCmd);
			return;
		}

		cmdParseError = false;
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("UserCommandRunner::RunCommand exception in user command parsing: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("UserCommandRunner::RunCommand unknown exception in user command parsing");
	}

	if(cmdParseError) {
		errorInfo = ErrorInvalidJsonUserCommand;
		return;
	}

	//expand user command to console commands
	if(_userCommand.command == UserCmdConnectDevice)
	{
		if(expandUserCmdConnectDevice()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingConnectDevice;
		}
	}
	else if(_userCommand.command == UserCmdResetDevice)
	{
		if(expandUserCmdResetDevice()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingResetDevice;
		}
	}
	else if(_userCommand.command == UserCmdInsertSmartCard)
	{
		if(expandUserCmdInsertSmartCard()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingInsertSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdRemoveSmartCard)
	{
		if(expandUserCmdRemoveSmartCard()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingRemoveSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdSwipeSmartCard)
	{
		if(expandUserCmdSwipeSmartCard()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingSwipeSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdTapSmartCard)
	{
		if(expandUserCmdTapSmartCard()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingTapSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdShowBarCode)
	{
		if(expandUserCmdShowBarCode()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingShowBarCode;
		}
	}
	else if(_userCommand.command == UserCmdPressPedKey)
	{
		if(expandUserCmdPressPedKey()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressPedKey;
		}
	}
	else if(_userCommand.command == UserCmdPressSoftKey)
	{
		if(expandUserCmdPressSoftKey()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressSoftKey;
		}
	}
	else if(_userCommand.command == UserCmdPressAssistKey)
	{
		if(expandUserCmdPressAssistKey()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressAssistKey;
		}
	}
	else if(_userCommand.command == UserCmdTouchScreen)
	{
		if(expandUserCmdTouchScreen()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingTouchScreen;
		}
	}
}

void UserCommandRunner::AddObserver(IUserCommandRunnerObserver * pObserver)
{
	Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex);

	if(pObserver != nullptr) {
		_observerPtrArray.push_back(pObserver);
	}
}

void UserCommandRunner::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			CommandState userCmdState;
			CommandState consoleCmdState;

			{
				Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex); //lock user cmd mutex
				userCmdState = _userCommand.state;
			}
			if(userCmdState == CommandState::Idle)
			{
				sleep(10);
				continue;
			}

			{
				Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex
				consoleCmdState = _consoleCommand.state;
			}
			switch(consoleCmdState)
			{
				case CommandState::Idle:
				{
					//run a console command
					std::string consoleCmd;
					Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex); //lock user cmd mutex

					_consoleCommand.state = CommandState::OnGoing; //change state here to give a correct state if callback comes instantly.
					consoleCmd = _userCommand.consoleCommands.front();
					pLogger->LogInfo("UserCommandRunner::runTask run console cmd: " + consoleCmd);
					_consoleCommand.cmdId = _pConsoleOperator->RunConsoleCommand(consoleCmd);

					if(_consoleCommand.cmdId == ICommandReception::ICommandDataTypes::InvalidCommandId)
					{
						//failed to run the console command
						pLogger->LogError("UserCommandRunner::runTask failed to run console cmd: " + consoleCmd);
						notifyObservers(_userCommand.commandId, CommandState::Failed, ErrorFailedToRunConsoleCommand);

						_userCommand.consoleCommands.clear();
						_userCommand.state = CommandState::Idle;

						Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex
						_consoleCommand.state = CommandState::Idle;
					}
				}
				break;

				case CommandState::OnGoing:
				{
					sleep(10);
				}
				break;

				case CommandState::Succeeded:
				{
					//pop up the console command
					std::string cmd = _userCommand.consoleCommands.front();
					pLogger->LogInfo("UserCommandRunner::runTask succeeded in console command: " + cmd);
					_userCommand.consoleCommands.pop_front();

					Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex
					_consoleCommand.state = CommandState::Idle;

					if(_userCommand.consoleCommands.empty())
					{
						//run out of console commands
						std::string empty;

						pLogger->LogInfo("UserCommandRunner::runTask succeeded in user command id: " + _userCommand.commandId);
						notifyObservers(_userCommand.commandId, CommandState::Succeeded, empty);

						Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex); //lock user cmd mutex
						_userCommand.state = CommandState::Idle;
					}
				}
				break;

				case CommandState::Failed:
				{
					Poco::ScopedLock<Poco::Mutex> consoleLock(_consoleCommandMutex); //lock console cmd mutex
					_consoleCommand.state = CommandState::Idle;

					std::string cmd = _userCommand.consoleCommands.front();
					pLogger->LogError("UserCommandRunner::runTask failed in console command: " + cmd);

					pLogger->LogError("UserCommandRunner::runTask failed in user command id: " + _userCommand.commandId);
					notifyObservers(_userCommand.commandId, CommandState::Failed, ErrorFailedToRunUserCommand);

					Poco::ScopedLock<Poco::Mutex> userLock(_userCommandMutex); //lock user cmd mutex
					_userCommand.consoleCommands.clear();
					_userCommand.state = CommandState::Idle;
				}
				break;

				default:
				{
					pLogger->LogError("UserCommandRunner::runTask wrong console command state: " + std::to_string((int)consoleCmdState));
				}
				break;
			}
		}
	}

	pLogger->LogInfo("UserCommandRunner::runTask exited");
}
