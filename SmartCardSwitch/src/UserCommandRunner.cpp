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

void UserCommandRunner::finishUserCommand(CommandState consoleCmdState, const std::string& errorInfo)
{
	CommandState userCmdState;
	std::string error = errorInfo;

	if(consoleCmdState != CommandState::Succeeded)
	{
		userCmdState = CommandState::Failed;
	}
	else
	{
		if(_userCommand.command == UserCmdConnectDevice)
		{
			if(_consoleCommand.resultDevicePowered == false) {
				userCmdState = CommandState::Failed;
				error = ErrorDeviceNotPowered;
			}
			else if(_consoleCommand.resultDevices.empty()) {
				userCmdState = CommandState::Failed;
				error = ErrorDeviceNotAvailable;
			}
			else if(_userCommand.deviceName != _consoleCommand.resultDevices[0]) {
				userCmdState = CommandState::Failed;
				error = ErrorDeviceNotAvailable;
			}
			else {
				userCmdState = CommandState::Succeeded;
			}
		}
		else if(_userCommand.command == UserCmdConfirmReset)
		{
			if(_consoleCommand.resultLocators[_userCommand.locatorIndexReset] == _userCommand.locatorIndexReset) {
				userCmdState = CommandState::Succeeded;
			}
			else {
				userCmdState = CommandState::Failed;
			}
		}
		else
		{
			userCmdState = CommandState::Succeeded;
		}
	}

	notifyObservers(_userCommand.commandId, userCmdState, error);
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
	cmd = ConsoleCommandFactory::CmdDeviceConnect(0);//only one device at the moment.
	_userCommand.consoleCommands.push_back(cmd);
	//queries
	cmd = ConsoleCommandFactory::CmdDeviceQueryPower();//device power
	_userCommand.consoleCommands.push_back(cmd);

	return true;
}

void UserCommandRunner::parseUserCmdConfirmReset(Poco::DynamicStruct& ds)
{
	unsigned int locatorIndex = ds["locatorIndex"];
	unsigned int lineNumber = ds["lineNumber"];

	_userCommand.locatorIndexReset = locatorIndex;
	_userCommand.lineNumberReset = lineNumber;
}

bool UserCommandRunner::expandUserCmdConfirmReset()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdLocatorQuery(_userCommand.locatorIndexReset);
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

		if(_userCommand.command == UserCmdConnectDevice) {
			parseUserCmdConnectDevice(ds);
		}
		else if(_userCommand.command == UserCmdConfirmReset) {
			parseUserCmdConfirmReset(ds);
		}
		else if(_userCommand.command == UserCmdResetDevice) {
			parseUserCmdResetDevice(ds);
		}
		else if(_userCommand.command == UserCmdInsertSmartCard) {
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
	else if(_userCommand.command == UserCmdConfirmReset)
	{
		if(expandUserCmdConfirmReset()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingConfirmReset;
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

void UserCommandRunner::OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnDevicesGet successful command Id: " + std::to_string(_consoleCommand.cmdId));

		_consoleCommand.resultDevices.clear();
		for(auto it=devices.begin(); it!=devices.end(); it++) {
			_consoleCommand.resultDevices.push_back(*it);
		}

		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDevicesGet failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnDeviceConnect(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnDeviceConnect successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDeviceConnect failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnDeviceConnect successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultDevicePowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDeviceConnect failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultDevicePowered = false;
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnDeviceQueryFuse successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultDeviceFuseOk = bFuseOn;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDeviceQueryFuse failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultDeviceFuseOk = false;
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnOptPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnOptPowerOn successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultOptPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnOptPowerOn failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnOptPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnOptPowerOff successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
		_consoleCommand.resultOptPowered = false;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnOptPowerOff failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnOptQueryPower successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultOptPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnOptQueryPower failure command Id: " + std::to_string(_consoleCommand.cmdId));
		//_consoleCommand.resultOptIsPowered = false;
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcsPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcsPowerOn successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcsPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcsPowerOn failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcsPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcsPowerOff successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcsPowered = false;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcsPowerOff failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcsQueryPower successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcsPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcsQueryPower failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcCoast(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcCoast successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcMode = BdcStatus::COAST;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcCoast failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcReverse(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcReverse successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcMode = BdcStatus::REVERSE;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcReverse failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcForward(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcForward successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcMode = BdcStatus::FORWARD;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcForward failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcBreak(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcBreak successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcMode = BdcStatus::BREAK;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcBreak failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnBdcQuery successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultBdcMode = status;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnBdcQuery failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnSteppersPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnSteppersPowerOn successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppersPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnSteppersPowerOn failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnSteppersPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnSteppersPowerOff successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppersPowered = false;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnSteppersPowerOff failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnSteppersQueryPower successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppersPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnSteppersQueryPower failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperConfigStep(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperConfigStep successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperConfigStep failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperAccelerationBuffer(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperAccelerationBuffer successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperAccelerationBuffer failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperAccelerationBufferDecrement successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperAccelerationBufferDecrement failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperDecelerationBuffer(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperDecelerationBuffer successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperDecelerationBuffer failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperDecelerationBufferIncrement successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperDecelerationBufferIncrement failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperEnable(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperEnable successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].enabled = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperEnable failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperForward(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperForward successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forward = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperForward failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperSteps(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperSteps successful command Id: " + std::to_string(_consoleCommand.cmdId));

		if(_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forward) {
			_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].targetPosition = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex].homeOffset + _consoleCommand.steps;
		}
		else {
			_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].targetPosition = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex].homeOffset - _consoleCommand.steps;
		}
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperSteps failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperRun(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperRun successful command Id: " + std::to_string(_consoleCommand.cmdId));

		auto& stepperData = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];
		switch(stepperData.state)
		{
			case StepperState::ApproachingHomeLocator:
			case StepperState::LeavingHomeLocator:
			case StepperState::GoingHome:
			{
				stepperData.state = StepperState::KnownPosition;
				stepperData.homeOffset = 0;
				pLogger->LogInfo("UserCommandRunner::OnStepperRun arrived at home position");
			}
			break;

			case StepperState::Accelerating:
			case StepperState::Cruising:
			case StepperState::Decelerating:
			case StepperState::KnownPosition:
			{
				stepperData.state = StepperState::KnownPosition;
				stepperData.homeOffset = stepperData.targetPosition;
				stepperData.targetPosition = 0;
				pLogger->LogInfo("UserCommandRunner::OnStepperRun arrived at: " + std::to_string(stepperData.homeOffset));
			}
			break;

			default:
			{
				pLogger->LogError("UserCommandRunner::OnStepperRun wrong stepper state: " + std::to_string((int)stepperData.state));
			}
			break;
		}
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperRun failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperConfigHome(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperConfigHome successful command Id: " + std::to_string(_consoleCommand.cmdId));

		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].state = StepperState::ApproachingHomeLocator;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperConfigHome failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnStepperQuery(CommandId key, bool bSuccess,
							StepperState state,
							bool bEnabled,
							bool bForward,
							unsigned int locatorIndex,
							unsigned int locatorLineNumberStart,
							unsigned int locatorLineNumberTerminal,
							unsigned long homeOffset,
							unsigned long lowClks,
							unsigned long highClks,
							unsigned long accelerationBuffer,
							unsigned long accelerationBufferDecrement,
							unsigned long decelerationBuffer,
							unsigned long decelerationBufferIncrement)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnStepperQuery successful command Id: " + std::to_string(_consoleCommand.cmdId));

		auto& stepperData = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];

		stepperData.enabled = bEnabled;
		stepperData.forward = bForward;
		stepperData.locatorIndex = locatorIndex;
		stepperData.locatorLineNumberStart = locatorLineNumberStart;
		stepperData.locatorLineNumberTerminal = locatorLineNumberTerminal;
		stepperData.homeOffset = homeOffset;

		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperQuery failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput)
{
	Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		pLogger->LogInfo("UserCommandRunner::OnLocatorQuery successful command Id: " + std::to_string(_consoleCommand.cmdId));

		_consoleCommand.resultLocators[_consoleCommand.locatorIndex] = lowInput;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnLocatorQuery failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
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
					Poco::ScopedLock<Poco::Mutex> userLock(_userCommandMutex); //lock user cmd mutex
					Poco::ScopedLock<Poco::Mutex> consoleLock(_consoleCommandMutex); //lock console cmd mutex

					//run a console command
					std::string consoleCmd;

					_consoleCommand.state = CommandState::OnGoing; //change state here to give a correct state if callback comes instantly.
					consoleCmd = _userCommand.consoleCommands.front();
					pLogger->LogInfo("UserCommandRunner::runTask run console cmd: " + consoleCmd);
					_consoleCommand.cmdId = _pConsoleOperator->RunConsoleCommand(consoleCmd);
					pLogger->LogInfo("UserCommandRunner::runTask command Id: " + std::to_string(_consoleCommand.cmdId));

					if(_consoleCommand.cmdId == ICommandReception::ICommandDataTypes::InvalidCommandId)
					{
						//failed to run the console command
						pLogger->LogError("UserCommandRunner::runTask failed to run console cmd: " + consoleCmd);
						finishUserCommand(CommandState::Failed, ErrorFailedToRunConsoleCommand);

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
						finishUserCommand(CommandState::Succeeded, empty);

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
					finishUserCommand(CommandState::Failed, ErrorFailedToRunUserCommand);

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
