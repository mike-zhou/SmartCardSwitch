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
			strState = "succeeded";
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

void UserCommandRunner::finishUserCommandConnectDevice(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	if(_consoleCommand.resultDevicePowered == false) {
		updatedCmdState = CommandState::Failed;
		updatedErrorInfo = ErrorDeviceNotPowered;
	}
	else if(_consoleCommand.resultDevices.empty()) {
		updatedCmdState = CommandState::Failed;
		updatedErrorInfo = ErrorDeviceNotAvailable;
	}
	else if(_userCommand.deviceName != _consoleCommand.resultDevices[0]) {
		updatedCmdState = CommandState::Failed;
		updatedErrorInfo = ErrorDeviceNotAvailable;
	}
	else {
		updatedCmdState = CommandState::Succeeded;
	}
}

void UserCommandRunner::finishUserCommandCheckResetPressed(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	if(_consoleCommand.resultLocators[_userCommand.locatorIndexReset] == _userCommand.lineNumberReset) {
		updatedCmdState = CommandState::Succeeded;
	}
	else {
		updatedCmdState = CommandState::Failed;
		updatedErrorInfo = ErrorResetIsNotPressed;
	}
}

void UserCommandRunner::finishUserCommandCheckResetReleased(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	if(_consoleCommand.resultLocators[_userCommand.locatorIndexReset] != _userCommand.lineNumberReset) {
		updatedCmdState = CommandState::Succeeded;
	}
	else {
		updatedCmdState = CommandState::Failed;
		updatedErrorInfo = ErrorResetIsNotReleased;
	}
}

void UserCommandRunner::finishUserCommandResetDevice(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	updatedCmdState = CommandState::Failed;

	if(!_consoleCommand.resultSteppersPowered) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice steppers are not powered");
		updatedErrorInfo = ErrorSteppersNotPoweredAfterReset;
	}
	else if(!_consoleCommand.resultBdcsPowered) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice steppers are not powered");
		updatedErrorInfo = ErrorBdcsNotPoweredAfterReset;
	}
	else if(_consoleCommand.resultSteppers[0].homeOffset != 0) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice stepper 0 not home positioned: " + std::to_string(_consoleCommand.resultSteppers[0].homeOffset));
		updatedErrorInfo = ErrorDeviceNotHomePositioned;
	}
	else if(_consoleCommand.resultSteppers[1].homeOffset != 0) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice stepper 1 not home positioned: " + std::to_string(_consoleCommand.resultSteppers[1].homeOffset));
		updatedErrorInfo = ErrorDeviceNotHomePositioned;
	}
	else if(_consoleCommand.resultSteppers[2].homeOffset != 0) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice stepper 2 not home positioned: " + std::to_string(_consoleCommand.resultSteppers[2].homeOffset));
		updatedErrorInfo = ErrorDeviceNotHomePositioned;
	}
	else if(_consoleCommand.resultSteppers[3].homeOffset != 0) {
		pLogger->LogError("UserCommandRunner::finishUserCommandResetDevice stepper 3 not home positioned: " + std::to_string(_consoleCommand.resultSteppers[3].homeOffset));
		updatedErrorInfo = ErrorDeviceNotHomePositioned;
	}
	else {
		pLogger->LogInfo("UserCommandRunner::finishUserCommandResetDevice home offset: "
				+ std::to_string(_consoleCommand.resultSteppers[0].homeOffset)
				+ ", " + std::to_string(_consoleCommand.resultSteppers[1].homeOffset)
				+ ", " + std::to_string(_consoleCommand.resultSteppers[2].homeOffset)
				+ ", " + std::to_string(_consoleCommand.resultSteppers[3].homeOffset));

		updatedCmdState = CommandState::Succeeded;
		_userCommand.smartCardReaderSlotOccupied = false;
	}
}

void UserCommandRunner::finishUserCommandInsertSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	_userCommand.smartCardReaderSlotOccupied = true;
}

void UserCommandRunner::finishUserCommandRemoveSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	_userCommand.smartCardReaderSlotOccupied = false;
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
			finishUserCommandConnectDevice(userCmdState, error);
		}
		else if(_userCommand.command == UserCmdCheckResetPressed)
		{
			finishUserCommandCheckResetPressed(userCmdState, error);
		}
		else if(_userCommand.command == UserCmdCheckResetReleased)
		{
			finishUserCommandCheckResetReleased(userCmdState, error);
		}
		else if(_userCommand.command == UserCmdResetDevice)
		{
			finishUserCommandResetDevice(userCmdState, error);
		}
		else if(_userCommand.command == UserCmdInsertSmartCard)
		{
			finishUserCommandInsertSmartCard(userCmdState, error);
		}
		else if(_userCommand.command == UserCmdRemoveSmartCard)
		{
			finishUserCommandRemoveSmartCard(userCmdState, error);
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

	_userCommand.consoleCommands.clear();

	cmd = ConsoleCommandFactory::CmdDevicesGet();
	_userCommand.consoleCommands.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdDeviceConnect(0);//only one device at the moment.
	_userCommand.consoleCommands.push_back(cmd);
	//queries
	cmd = ConsoleCommandFactory::CmdDeviceQueryPower();//device power
	_userCommand.consoleCommands.push_back(cmd);

	return true;
}

void UserCommandRunner::parseUserCmdCheckResetPressed(Poco::DynamicStruct& ds)
{
	unsigned int locatorIndex = ds["locatorIndex"];
	unsigned int lineNumber = ds["lineNumber"];

	_userCommand.locatorIndexReset = locatorIndex;
	_userCommand.lineNumberReset = lineNumber;
}

bool UserCommandRunner::expandUserCmdCheckResetPressed()
{
	std::string cmd;

	_userCommand.consoleCommands.clear();

	cmd = ConsoleCommandFactory::CmdLocatorQuery(_userCommand.locatorIndexReset);
	_userCommand.consoleCommands.push_back(cmd);

	_consoleCommand.locatorIndex = _userCommand.locatorIndexReset;// is there another way not to touch consoleCommand?

	return true;
}

void UserCommandRunner::parseUserCmdCheckResetReleased(Poco::DynamicStruct& ds)
{
	unsigned int locatorIndex = ds["locatorIndex"];
	unsigned int lineNumber = ds["lineNumber"];

	_userCommand.locatorIndexReset = locatorIndex;
	_userCommand.lineNumberReset = lineNumber;
}

bool UserCommandRunner::expandUserCmdCheckResetReleased()
{
	std::string cmd;

	_userCommand.consoleCommands.clear();

	cmd = ConsoleCommandFactory::CmdLocatorQuery(_userCommand.locatorIndexReset);
	_userCommand.consoleCommands.push_back(cmd);

	_consoleCommand.locatorIndex = _userCommand.locatorIndexReset; // is there another way not to touch consoleCommand?

	return true;
}

void UserCommandRunner::parseUserCmdResetDevice(Poco::DynamicStruct& ds)
{
	//nothing further to be done
}

void UserCommandRunner::configStepperMovement(unsigned int index,
											unsigned int lowClks,
											unsigned int highClks,
											unsigned int accelerationBuffer,
											unsigned int accelerationBufferDecrement,
											unsigned int decelerationBuffer,
											unsigned int decelerationBufferIncrement,
											std::vector<std::string>& cmds)
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdStepperConfigStep(index, lowClks, highClks);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperAccelerationBuffer(index, accelerationBuffer);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperAccelerationBufferDecrement(index, accelerationBufferDecrement);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperDecelerationBuffer(index, decelerationBuffer);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperDecelerationBufferIncrement(index, decelerationBufferIncrement);
	cmds.push_back(cmd);
}

bool UserCommandRunner::expandUserCmdResetDevice()
{
	std::string cmd;
	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;
	int locatorIndex;
	int locatorLineNumberStart;
	int locatorLineNumberTerminal;

	const unsigned int x = 0;
	const unsigned int y = 1;
	const unsigned int z = 2;
	const unsigned int w = 3;


	_userCommand.consoleCommands.clear();

	//power on steppers
	cmd = ConsoleCommandFactory::CmdSteppersPowerOn();
	_userCommand.consoleCommands.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdSteppersQueryPower();
	_userCommand.consoleCommands.push_back(cmd);
	//power on BDCs
	cmd = ConsoleCommandFactory::CmdBdcsPowerOn();
	_userCommand.consoleCommands.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdBdcsQueryPower();
	_userCommand.consoleCommands.push_back(cmd);

	//configure steppers
	// z
	{
		unsigned int i = z;
		auto rc = pMovementConfiguration->GetStepperGoHome(lowClks,
															highClks,
															accelerationBuffer,
															accelerationBufferDecrement,
															decelerationBuffer,
															decelerationBufferIncrement);
		rc = rc && pMovementConfiguration->GetStepperBoundary(i,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc)
		{
			std::vector<std::string> cmds;

			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement,
								cmds);
			for(auto it=cmds.begin(); it!=cmds.end(); it++) {
				_userCommand.consoleCommands.push_back(*it);
			}
			cmd = ConsoleCommandFactory::CmdStepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			_userCommand.consoleCommands.push_back(cmd);
			cmd = ConsoleCommandFactory::CmdStepperRun(i, 0, 0);
			_userCommand.consoleCommands.push_back(cmd);
		}
		else
		{
			pLogger->LogError("UserCommandRunner::expandUserCmdResetDevice failed to retrieve configuration for stepper: " + std::to_string(i));
			return false;
		}
	}
	// w
	{
		unsigned int i = w;
		auto rc = pMovementConfiguration->GetStepperBoundary(i,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc)
		{
			std::vector<std::string> cmds;

			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement,
								cmds);
			for(auto it=cmds.begin(); it!=cmds.end(); it++) {
				_userCommand.consoleCommands.push_back(*it);
			}
			cmd = ConsoleCommandFactory::CmdStepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			_userCommand.consoleCommands.push_back(cmd);
			cmd = ConsoleCommandFactory::CmdStepperRun(i, 0, 0);
			_userCommand.consoleCommands.push_back(cmd);
		}
		else
		{
			pLogger->LogError("UserCommandRunner::expandUserCmdResetDevice failed to retrieve configuration for stepper: " + std::to_string(i));
			return false;
		}
	}
	// y
	{
		unsigned int i = y;
		auto rc = pMovementConfiguration->GetStepperBoundary(i,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc)
		{
			std::vector<std::string> cmds;

			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement,
								cmds);
			for(auto it=cmds.begin(); it!=cmds.end(); it++) {
				_userCommand.consoleCommands.push_back(*it);
			}
			cmd = ConsoleCommandFactory::CmdStepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			_userCommand.consoleCommands.push_back(cmd);
			cmd = ConsoleCommandFactory::CmdStepperRun(i, 0, 0);
			_userCommand.consoleCommands.push_back(cmd);
		}
		else
		{
			pLogger->LogError("UserCommandRunner::expandUserCmdResetDevice failed to retrieve configuration for stepper: " + std::to_string(i));
			return false;
		}
	}
	// x
	{
		unsigned int i = x;
		auto rc = pMovementConfiguration->GetStepperBoundary(i,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc)
		{
			std::vector<std::string> cmds;

			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement,
								cmds);
			for(auto it=cmds.begin(); it!=cmds.end(); it++) {
				_userCommand.consoleCommands.push_back(*it);
			}
			cmd = ConsoleCommandFactory::CmdStepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			_userCommand.consoleCommands.push_back(cmd);
			cmd = ConsoleCommandFactory::CmdStepperRun(i, 0, 0);
			_userCommand.consoleCommands.push_back(cmd);
		}
		else
		{
			pLogger->LogError("UserCommandRunner::expandUserCmdResetDevice failed to retrieve configuration for stepper: " + std::to_string(i));
			return false;
		}
	}

	//apply general movement configuration
	for(unsigned int i = 0; i < STEPPER_AMOUNT; i++)
	{
		auto rc = pMovementConfiguration->GetStepperGeneral(i,
															lowClks,
															highClks,
															accelerationBuffer,
															accelerationBufferDecrement,
															decelerationBuffer,
															decelerationBufferIncrement);
		rc = rc && pMovementConfiguration->GetStepperBoundary(i,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc)
		{
			std::vector<std::string> cmds;

			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement,
								cmds);
			for(auto it=cmds.begin(); it!=cmds.end(); it++) {
				_userCommand.consoleCommands.push_back(*it);
			}
		}
		else
		{
			pLogger->LogError("UserCommandRunner::expandUserCmdResetDevice failed to retrieve general configuration for stepper: " + std::to_string(i));
			return false;
		}
	}

	//query steppers
	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		cmd = ConsoleCommandFactory::CmdStepperQuery(i);
		_userCommand.consoleCommands.push_back(cmd);
	}

	return true;
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

void UserCommandRunner::parseUserCmdSwipeSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		std::string err = "UserCommandRunner::parseUserCmdSwipeSmartCard smart card number of range: " + std::to_string(number);
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.smartCardNumber = number;
	_userCommand.downPeriod = ds["downPeriod"];
}

void UserCommandRunner::parseUserCmdTapSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		std::string err = "UserCommandRunner::parseUserCmdTapSmartCard smart card number of range: " + std::to_string(number);
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.smartCardNumber = number;
	_userCommand.downPeriod = ds["downPeriod"];
}

int UserCommandRunner::currentX()
{
	if(_consoleCommand.resultSteppers[0].state == StepperState::Unknown) {
		return -1;
	}

	return _consoleCommand.resultSteppers[0].homeOffset;
}

int UserCommandRunner::currentY()
{
	if(_consoleCommand.resultSteppers[1].state == StepperState::Unknown) {
		return -1;
	}

	return _consoleCommand.resultSteppers[1].homeOffset;
}

int UserCommandRunner::currentZ()
{
	if(_consoleCommand.resultSteppers[2].state == StepperState::Unknown) {
		return -1;
	}

	return _consoleCommand.resultSteppers[2].homeOffset;
}

int UserCommandRunner::currentW()
{
	if(_consoleCommand.resultSteppers[3].state == StepperState::Unknown) {
		return -1;
	}

	return _consoleCommand.resultSteppers[3].homeOffset;
}

UserCommandRunner::CurrentPosition UserCommandRunner::getCurrentPosition()
{
	int x, y, z, w;
	int curX, curY, curZ, curW;

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::Home, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::Home;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::SmartCardGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::PedKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::SoftKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::AssistKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::TouchScreenGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::SmartCardReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::BarCodeReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::ContactlessReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
	if((curX == x) && (curY == y) && (curZ == z) && (curW = w)) {
		return CurrentPosition::SmartCardGate;
	}

	return CurrentPosition::Unknown;
}

void UserCommandRunner::moveStepper(unsigned int index, unsigned int initialPos, unsigned int finalPos, std::vector<std::string>& cmds)
{
	if(index >= STEPPER_AMOUNT) {
		pLogger->LogError("UserCommandRunner::moveStepper stepper index out of range: " + std::to_string(index));
		return;
	}
	if(initialPos == finalPos) {
		return;
	}

	std::string cmd;
	bool forward = (finalPos > initialPos);
	unsigned int steps;

	if(forward) {
		steps = finalPos - initialPos;
	}
	else {
		steps = initialPos - finalPos;
	}

	cmd = ConsoleCommandFactory::CmdStepperForward(index, forward);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperSteps(index, steps);
	cmds.push_back(cmd);
	cmd = ConsoleCommandFactory::CmdStepperRun(index, initialPos, finalPos);
	cmds.push_back(cmd);
}

std::vector<std::string> UserCommandRunner::toHome()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toHome unknown current position");
	}
	else if(currentPosition != CurrentPosition::Home)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		//move to home position
		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::Home, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toSmartCardGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toSmartCardGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::SmartCardGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toPedKeyGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toPedKeyGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::PedKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toSoftKeyGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toSoftKeyGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::SoftKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toAssistKeyGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toAssistKeyGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::AssistKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toTouchScreenGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toTouchScreenGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::TouchScreenGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toSmartCardReaderGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toSmartCardReaderGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::SmartCardReaderGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up to cruise height
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, x, y, z, w);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperW(curW, w, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toContactlessReaderGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toContactlessReaderGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::ContactlessReaderGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::toBarcodeReaderGate()
{
	auto currentPosition = getCurrentPosition();
	std::vector<std::string> cmds;

	if(currentPosition == CurrentPosition::Unknown)
	{
		pLogger->LogError("UserCommandRunner::toBarcodeReaderGate unknown current position");
	}
	else if(currentPosition != CurrentPosition::BarCodeReaderGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;
		std::string cmd;

		//move up
		curZ = currentZ();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
		moveStepperZ(curZ, z, cmds);

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, x, y, z, w);
		moveStepperW(curW, w, cmds);
		moveStepperY(curY, y, cmds);
		moveStepperX(curX, x, cmds);
		moveStepperZ(curZ, z, cmds);
	}

	return cmds;
}

std::vector<std::string> UserCommandRunner::gate_smartCard_withoutCard(unsigned int cardNumber)
{
	std::string cmd;
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long offset;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve smart card gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, finalX, finalY, finalZ, finalW, cardNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve smart card: " + std::to_string(cardNumber));
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve fetch offset");
		return result;
	}

	//move to smart card
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(curY, finalY + offset, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//move down
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(finalY + offset, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::smartCard_gate_withCard(unsigned int cardNumber)
{
	std::string cmd;
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCard_gate_withCard failed to retrieve smart card gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, curX, curY, curZ, curW, cardNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCard_gate_withCard failed to retrieve smart card: " + std::to_string(cardNumber));
		return result;
	}

	//Z
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//Y
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//X
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_smartCardReader_withCard()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long slowInsertEnd;


	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader gate");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardReaderSlowInsertEndY(slowInsertEnd);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader slow insert end");
		return result;
	}
	rc = pMovementConfiguration->GetStepperCardInsert(lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve stepper card slow insert");
		return result;
	}

	//configure movement
	cmds.clear();
	configStepperMovement(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	//slow insertion
	cmds.clear();
	moveStepperY(curY, slowInsertEnd, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//restore to normal speed
	rc = pMovementConfiguration->GetStepperGeneral(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve stepper general");
		result.clear();
		return result;
	}
	//configure movement
	cmds.clear();
	configStepperMovement(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	//insert card
	cmds.clear();
	moveStepperY(slowInsertEnd, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::smartCardReader_gate_withCard()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCardReader_gate_withCard failed to retrieve smart card reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCardReader_gate_withCard failed to retrieve smart card reader gate");
		return result;
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::smartCardReader_gate_withoutCard()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long fetchOffset;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve smart card reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve smart card reader gate");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(fetchOffset);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve fetchOffset");
		return result;
	}

	cmds.clear();
	moveStepperZ(curZ, curZ - fetchOffset, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperZ(curZ - fetchOffset, curZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::openClamp()
{
	unsigned long lowClks, highClks, cycles;
	std::vector<std::string> result;
	std::string cmd;

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	cmd = ConsoleCommandFactory::CmdBdcReverse(0, lowClks, highClks, cycles);
	result.push_back(cmd);

	return result;
}

std::vector<std::string> UserCommandRunner::closeClamp()
{
	unsigned long lowClks, highClks, cycles;
	std::vector<std::string> result;
	std::string cmd;

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	cmd = ConsoleCommandFactory::CmdBdcForward(0, lowClks, highClks, cycles);
	result.push_back(cmd);

	return result;
}

std::vector<std::string> UserCommandRunner::releaseClamp()
{
	std::vector<std::string> result;
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdBdcCoast(0);
	result.push_back(cmd);

	return result;
}

bool UserCommandRunner::expandUserCmdInsertSmartCard()
{
	std::vector<std::string> cmds;

	//check if smart card slot is empty
	if(_userCommand.smartCardReaderSlotOccupied) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard card in smart card reader");
		return false;
	}

	_userCommand.consoleCommands.clear();

	//to smart card gate
	cmds = toSmartCardGate();
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to gate with card
	cmds.clear();
	cmds = smartCard_gate_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//to smart card reader gate
	cmds.clear();
	cmds = toSmartCardReaderGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//insert card
	cmds.clear();
	cmds = gate_smartCardReader_withCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card reader gate
	cmds.clear();
	cmds = smartCardReader_gate_withoutCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//release clamp
	cmds.clear();
	cmds = releaseClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdInsertSmartCard failed to expand user command");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::gate_smartCardReader_withoutCard()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long offset;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card reader gate");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card offset");
		return result;
	}

	cmds.clear();
	moveStepperZ(curZ, curZ - offset, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperZ(curZ + offset, curZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_smartCard_withCard(unsigned int cardNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long placeStart;
	long releaseOffset;


	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, finalX, finalY, finalZ, finalW, _userCommand.smartCardNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card gate");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardPlaceStartZ(placeStart);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card place start");
		return result;
	}
	rc = pMovementConfiguration->GetStepperCardInsert(lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve stepper card slow insert");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardReleaseOffset(releaseOffset);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card release offset");
		return result;
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperZ(curZ, placeStart, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperZ(placeStart, finalZ + releaseOffset, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	rc = pMovementConfiguration->GetStepperGeneral(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		result.clear();
		pLogger->LogError("UserCommandRunner::gate_smartCard_withCard failed to retrieve stepper card slow insert");
		return result;
	}
	cmds.clear();
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::smartCard_gate_withoutCard(unsigned int cardNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long offset;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, curX, curY, curZ, curW, _userCommand.smartCardNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card gate");
		return result;
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card fetch offset");
		return result;
	}

	//in order not to interfere card in smart card bay
	cmds.clear();
	moveStepperY(curY, curY + offset, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperY(curY + offset, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

bool UserCommandRunner::expandUserCmdRemoveSmartCard()
{
	std::vector<std::string> cmds;

	//check if smart card slot is empty
	if(!_userCommand.smartCardReaderSlotOccupied) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard no card in smart card reader");
		return false;
	}

	_userCommand.consoleCommands.clear();

	//to smart card reader gate
	cmds = toSmartCardReaderGate();
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card reader
	cmds.clear();
	cmds = gate_smartCardReader_withoutCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in gate_smartCardReader_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in closeClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card reader gate
	cmds.clear();
	cmds = smartCardReader_gate_withCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in smartCardReader_gate_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = toSmartCardGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in toSmartCardGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in gate_smartCard_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in smartCard_gate_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//release clamp
	cmds.clear();
	cmds = releaseClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdRemoveSmartCard failed in releaseClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

bool UserCommandRunner::expandUserCmdSwipeSmartCard()
{
	std::vector<std::string> cmds;

	//check if smart card slot is empty
	if(_userCommand.smartCardReaderSlotOccupied) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard card in smart card reader");
		return false;
	}

	_userCommand.consoleCommands.clear();

	//to smart card gate
	cmds = toSmartCardGate();
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in gate_smartCard_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in closeClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to gate with card
	cmds.clear();
	cmds = smartCard_gate_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in smartCard_gate_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//to smart card reader gate
	cmds.clear();
	cmds = toSmartCardReaderGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in toSmartCardReaderGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//insert card
	cmds.clear();
	cmds = gate_smartCardReader_withCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in gate_smartCardReader_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open and close clamp to clear any offset error.
	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}
	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in closeClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card reader gate
	cmds.clear();
	cmds = smartCardReader_gate_withCard();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in smartCardReader_gate_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = toSmartCardGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in toSmartCardGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in gate_smartCard_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in smartCard_gate_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//release clamp
	cmds.clear();
	cmds = releaseClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdSwipeSmartCard failed in release Clamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::deviceDelay(unsigned int clks)
{
	std::string cmd;
	std::vector<std::string> result;

	cmd = ConsoleCommandFactory::CmdDeviceDelay(clks);

	result.push_back(cmd);

	return result;
}

std::vector<std::string> UserCommandRunner::gate_contactlessReader()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_contactlessReader failed to retrieve contactless reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_contactlessReader failed to retrieve contactless reader gate");
		return result;
	}

	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::contactlessReader_gate()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::contactlessReader_gate failed to retrieve contactless reader gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReader, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::contactlessReader_gate failed to retrieve contactless reader");
		return result;
	}

	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

bool UserCommandRunner::expandUserCmdTapSmartCard()
{
	std::vector<std::string> cmds;

	_userCommand.consoleCommands.clear();

	//to smart card gate
	cmds = toSmartCardGate();
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in gate_smartCard_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in closeClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to gate with card
	cmds.clear();
	cmds = smartCard_gate_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in smartCard_gate_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	cmds.clear();
	cmds = toContactlessReaderGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in toContactlessReaderGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	cmds.clear();
	cmds = gate_contactlessReader();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in gate_contactlessReader");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in deviceDelay");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	cmds.clear();
	cmds = contactlessReader_gate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in contactlessReader_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	cmds = toSmartCardGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in toSmartCardGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in gate_smartCard_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in smartCard_gate_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//release clamp
	cmds.clear();
	cmds = releaseClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTapSmartCard failed in releaseClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

void UserCommandRunner::parseUserCmdBarCode(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];
	unsigned int downPeriod = ds["downPeriod"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		std::string err = "UserCommandRunner::parseUserCmdShowBarCode bar code card number of range: " + std::to_string(number);
		pLogger->LogError(err);
		throw Poco::Exception(err);
	}

	_userCommand.smartCardNumber = number;
	_userCommand.downPeriod = downPeriod;
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
				std::string err = "UserCommandRunner::parseUserCmdKeys key number of range: " + std::to_string(number);
				pLogger->LogError(err);
				throw Poco::Exception(err);
			}
		}
		else {
			std::string err = "UserCommandRunner::parseUserCmdKeys index out of range: " + std::to_string(index);
			pLogger->LogError(err);
			throw Poco::Exception(err);
		}
	}
}

std::vector<std::string> UserCommandRunner::barcodeReader_gate()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::barcodeReader_gate failed to retrieve bar code reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReader, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::barcodeReader_gate failed to retrieve bar code reader gate");
		return result;
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_barcodeReader()
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_barcodeReader failed to retrieve bar code reader");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_barcodeReader failed to retrieve bar code reader gate");
		return result;
	}

	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

bool UserCommandRunner::expandUserCmdShowBarCode()
{
	std::vector<std::string> cmds;

	_userCommand.consoleCommands.clear();

	//to smart card gate
	cmds = toSmartCardGate();
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in gate_smartCard_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//close clamp
	cmds.clear();
	cmds = closeClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in closeClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to gate with card
	cmds.clear();
	cmds = smartCard_gate_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in smartCard_gate_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//to smart card reader gate
	cmds.clear();
	cmds = toSmartCardReaderGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in toSmartCardReaderGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//to bar code reader gate
	cmds.clear();
	cmds = toBarcodeReaderGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in toBarcodeReaderGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//gate to bar code reader
	cmds.clear();
	cmds = gate_barcodeReader();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in gate_barcodeReader");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in deviceDelay");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//bar code reader to gate
	cmds.clear();
	cmds = barcodeReader_gate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in barcodeReader_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//to smart card gate
	cmds.clear();
	cmds = toSmartCardGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in toSmartCardGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card
	cmds.clear();
	cmds = gate_smartCard_withCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in gate_smartCard_withCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//open clamp
	cmds.clear();
	cmds = openClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in openClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//move to smart card gate
	cmds.clear();
	cmds = smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in smartCard_gate_withoutCard");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//release clamp
	cmds.clear();
	cmds = releaseClamp();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdShowBarCode failed in releaseClamp");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::pedKey_gate(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key:" + std::to_string(keyNumber));
		return result;
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::pedKey_pedKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key: " + std::to_string(keyNumberTo));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key pressed: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_pedKey(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key: " + std::to_string(keyNumber));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key pressed: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_pedKey failed to retrieve ped key: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}


bool UserCommandRunner::expandUserCmdPressPedKey()
{
	std::vector<std::string> cmds;

	if(_userCommand.keyNumbers.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressPedKey no key to press");
		return false;
	}
	_userCommand.consoleCommands.clear();

	cmds = toPedKeyGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressPedKey failed in toPedKeyGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;
	//press first key
	cmds.clear();
	cmds = gate_pedKey(pKeys[0]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressPedKey failed in gate_pedKey");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		cmds.clear();
		cmds = pedKey_pedKey(pKeys[i], pKeys[i+1]);
		if(cmds.empty()) {
			pLogger->LogError("UserCommandRunner::expandUserCmdPressPedKey failed in pedKey_pedKey: " + std::to_string(pKeys[i]) + " to " + std::to_string(pKeys[i+1]));
			return false;
		}
		for(auto it=cmds.begin(); it!=cmds.end(); it++) {
			_userCommand.consoleCommands.push_back(*it);
		}
	}

	//back to gate
	cmds.clear();
	cmds = pedKey_gate(pKeys[lastKeyIndex]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressPedKey failed in pedKey_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::softKey_gate(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_gate failed to retrieve soft key gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_gate failed to retrieve soft key:" + std::to_string(keyNumber));
		return result;
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::softKey_softKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_softKey failed to retrieve soft key: " + std::to_string(keyNumberTo));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_softKey failed to retrieve soft key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_softKey failed to retrieve soft key pressed: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::softKey_softKey failed to retrieve soft key: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_softKey(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_softKey failed to retrieve soft key: " + std::to_string(keyNumber));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_softKey failed to retrieve soft key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_softKey failed to retrieve soft key pressed: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_softKey failed to retrieve soft key: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

bool UserCommandRunner::expandUserCmdPressSoftKey()
{
	std::vector<std::string> cmds;

	if(_userCommand.keyNumbers.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressSoftKey no key to press");
		return false;
	}
	_userCommand.consoleCommands.clear();

	cmds = toSoftKeyGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressSoftKey failed in toSoftKeyGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;
	//press first key
	cmds.clear();
	cmds = gate_softKey(pKeys[0]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressSoftKey failed in gate_softKey");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		cmds.clear();
		cmds = softKey_softKey(pKeys[i], pKeys[i+1]);
		if(cmds.empty()) {
			pLogger->LogError("UserCommandRunner::expandUserCmdPressSoftKey failed in softKey_softKey: " + std::to_string(pKeys[i]) + " to " + std::to_string(pKeys[i+1]));
			return false;
		}
		for(auto it=cmds.begin(); it!=cmds.end(); it++) {
			_userCommand.consoleCommands.push_back(*it);
		}
	}

	//back to gate
	cmds.clear();
	cmds = softKey_gate(pKeys[lastKeyIndex]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressSoftKey failed in softKey_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::assistKey_gate(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_gate failed to retrieve assist key gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_gate failed to retrieve assist key:" + std::to_string(keyNumber));
		return result;
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::assistKey_assistKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key: " + std::to_string(keyNumberTo));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key pressed: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_assistKey(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_assistKey failed to retrieve assist key: " + std::to_string(keyNumber));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_assistKey failed to retrieve assist key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_assistKey failed to retrieve assist key pressed: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_assistKey failed to retrieve assist key: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

bool UserCommandRunner::expandUserCmdPressAssistKey()
{
	std::vector<std::string> cmds;

	if(_userCommand.keyNumbers.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressAssistKey no key to press");
		return false;
	}
	_userCommand.consoleCommands.clear();

	cmds = toAssistKeyGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressAssistKey failed in toAssistKeyGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;
	//press first key
	cmds.clear();
	cmds = gate_assistKey(pKeys[0]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressAssistKey failed in gate_assistKey");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		cmds.clear();
		cmds = assistKey_assistKey(pKeys[i], pKeys[i+1]);
		if(cmds.empty()) {
			pLogger->LogError("UserCommandRunner::expandUserCmdPressAssistKey failed in assistKey_assistKey: " + std::to_string(pKeys[i]) + " to " + std::to_string(pKeys[i+1]));
			return false;
		}
		for(auto it=cmds.begin(); it!=cmds.end(); it++) {
			_userCommand.consoleCommands.push_back(*it);
		}
	}

	//back to gate
	cmds.clear();
	cmds = assistKey_gate(pKeys[lastKeyIndex]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdPressAssistKey failed in assistKey_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

std::vector<std::string> UserCommandRunner::touchScreenKey_gate(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_gate failed to retrieve touch screen key gate");
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_gate failed to retrieve touch screen key:" + std::to_string(keyNumber));
		return result;
	}

	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::touchScreenKey_touchScreenKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumberTo));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key pressed: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumberTo));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}

std::vector<std::string> UserCommandRunner::gate_touchScreenKey(unsigned int keyNumber)
{
	std::vector<std::string> cmds;
	std::vector<std::string> result;

	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumber));
		return result;
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key gate");
		return result;
	}

	//to key
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key pressed: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.downPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		pLogger->LogError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumber));
		result.clear();
		return result;
	}
	cmds.clear();
	moveStepperZ(curZ, finalZ, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperY(curY, finalY, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}
	cmds.clear();
	moveStepperX(curX, finalX, cmds);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	//delay
	cmds.clear();
	cmds = deviceDelay(_userCommand.upPeriod);
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		result.push_back(*it);
	}

	return result;
}


bool UserCommandRunner::expandUserCmdTouchScreen()
{
	std::vector<std::string> cmds;

	if(_userCommand.keyNumbers.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTouchScreen no key to press");
		return false;
	}
	_userCommand.consoleCommands.clear();

	cmds = toTouchScreenGate();
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTouchScreen failed in toTouchScreenGate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;
	//press first key
	cmds.clear();
	cmds = gate_touchScreenKey(pKeys[0]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTouchScreen failed in gate_touchScreenKey");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		cmds.clear();
		cmds = touchScreenKey_touchScreenKey(pKeys[i], pKeys[i+1]);
		if(cmds.empty()) {
			pLogger->LogError("UserCommandRunner::expandUserCmdTouchScreen failed in touchScreenKey_touchScreenKey: " + std::to_string(pKeys[i]) + " to " + std::to_string(pKeys[i+1]));
			return false;
		}
		for(auto it=cmds.begin(); it!=cmds.end(); it++) {
			_userCommand.consoleCommands.push_back(*it);
		}
	}

	//back to gate
	cmds.clear();
	cmds = touchScreenKey_gate(pKeys[lastKeyIndex]);
	if(cmds.empty()) {
		pLogger->LogError("UserCommandRunner::expandUserCmdTouchScreen failed in touchScreenKey_gate");
		return false;
	}
	for(auto it=cmds.begin(); it!=cmds.end(); it++) {
		_userCommand.consoleCommands.push_back(*it);
	}

	return true;
}

void UserCommandRunner::RunCommand(const std::string& jsonCmd, std::string& errorInfo)
{
	Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex);

	pLogger->LogInfo("UserCommandRunner::RunCommand parse command: ====== " + jsonCmd);

//	if(_deviceHomePositioned == false) {
//		errorInfo = ErrorDeviceNotHomePositioned;
//		pLogger->LogError("UserCommandRunner::RunCommand device not home positioned, denied: " + jsonCmd);
//		return;
//	}

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

		//common data among all user command
		_userCommand.command = ds["userCommand"].toString();
		_userCommand.commandId = ds["commandId"].toString();

		//parse specific data in user command
		if(_userCommand.command == UserCmdConnectDevice) {
			parseUserCmdConnectDevice(ds);
		}
		else if(_userCommand.command == UserCmdCheckResetPressed) {
			parseUserCmdCheckResetPressed(ds);
		}
		else if(_userCommand.command == UserCmdCheckResetReleased) {
			parseUserCmdCheckResetReleased(ds);
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
			parseUserCmdSwipeSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdTapSmartCard) {
			parseUserCmdTapSmartCard(ds);
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

	//expand USER command to CONSOLE commands
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
	else if(_userCommand.command == UserCmdCheckResetPressed)
	{
		if(expandUserCmdCheckResetPressed()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingCheckResetPressed;
		}
	}
	else if(_userCommand.command == UserCmdCheckResetReleased)
	{
		if(expandUserCmdCheckResetReleased()) {
			_userCommand.state = CommandState::OnGoing;
			errorInfo.clear();
		}
		else {
			errorInfo = ErrorFailedExpandingCheckResetReleased;
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
		if(_userCommand.smartCardReaderSlotOccupied) {
			errorInfo = ErrorSmartCardReaderSlotOccupied;
		}
		else if(expandUserCmdInsertSmartCard()) {
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

void UserCommandRunner::SetConsoleOperator(ConsoleOperator * pCO)
{
	if(pCO == nullptr) {
		pLogger->LogError("UserCommandRunner::SetConsoleOperator invalid parameter");
		return;
	}
	_pConsoleOperator = pCO;
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

void UserCommandRunner::OnDeviceDelay(CommandId key, bool bSuccess)
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
		pLogger->LogInfo("UserCommandRunner::OnDeviceDelay successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDeviceDelay failure command Id: " + std::to_string(_consoleCommand.cmdId));
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

void UserCommandRunner::setConsoleCommandParameter(const std::string & cmd)
{
	unsigned int stepperIndex, locatorIndex, steps;

	if(ConsoleCommandFactory::GetParameterStepperIndex(cmd, stepperIndex)) {
		_consoleCommand.stepperIndex = stepperIndex;
	}

	if(ConsoleCommandFactory::GetParameterStepperSteps(cmd, steps)) {
		_consoleCommand.steps = steps;
	}

	if(ConsoleCommandFactory::GetParameterLocatorIndex(cmd, locatorIndex)) {
		_consoleCommand.locatorIndex = locatorIndex;
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
				// no user command is on-going
				sleep(10);
				continue;
			}

			//run console command
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
					pLogger->LogInfo("UserCommandRunner::runTask run console cmd: ------ " + consoleCmd);
					setConsoleCommandParameter(consoleCmd);
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
