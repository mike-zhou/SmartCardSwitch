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
	_state = IUserCommandRunnerObserver::State::Idle;
}

void UserCommandRunner::notifyObservers(const std::string& cmdId, IUserCommandRunnerObserver::State state, const std::string& errorInfo)
{
	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnCommandStatus(cmdId, state, errorInfo);
	}
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

	if(_state == IUserCommandRunnerObserver::State::OnGoing) {
		errorInfo = ErrorUserCommandOnGoing;
		pLogger->LogError("UserCommandRunner::RunCommand command ongoing, denied: " + jsonCmd);
		return;
	}

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

	if(_userCommand.command == UserCmdInsertSmartCard) {
		if(expandUserCmdInsertSmartCard()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingInsertSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdRemoveSmartCard) {
		if(expandUserCmdRemoveSmartCard()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingRemoveSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdSwipeSmartCard) {
		if(expandUserCmdSwipeSmartCard()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingSwipeSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdTapSmartCard) {
		if(expandUserCmdTapSmartCard()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingTapSmartCard;
		}
	}
	else if(_userCommand.command == UserCmdShowBarCode) {
		if(expandUserCmdShowBarCode()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingShowBarCode;
		}
	}
	else if(_userCommand.command == UserCmdPressPedKey) {
		if(expandUserCmdPressPedKey()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressPedKey;
		}
	}
	else if(_userCommand.command == UserCmdPressSoftKey) {
		if(expandUserCmdPressSoftKey()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressSoftKey;
		}
	}
	else if(_userCommand.command == UserCmdPressAssistKey) {
		if(expandUserCmdPressAssistKey()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingPressAssistKey;
		}
	}
	else if(_userCommand.command == UserCmdTouchScreen) {
		if(expandUserCmdTouchScreen()) {
			_state = IUserCommandRunnerObserver::State::OnGoing;
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

void UserCommandRunner::GetState(std::string& commandId, IUserCommandRunnerObserver::State& state, std::string& errorInfo)
{

}

void UserCommandRunner::runTask()
{

}
