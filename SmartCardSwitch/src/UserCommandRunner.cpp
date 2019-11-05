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
	_deviceHomePositioned = false;
	_clampState = ClampState::Released;
	_currentPosition = CoordinateStorage::Type::Home;
	_userCommand.state = CommandState::Idle;
	_userCommand.cardState = CardState::InBay;
	_userCommand.mobileBarcodeState = MobileBarcodeState::InMobileBarcodeBay;
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
		_userCommand.cardState = CardState::InBay;
	}
}

void UserCommandRunner::finishUserCommandInsertSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	_userCommand.cardState = CardState::InSmartCardReader;
}

void UserCommandRunner::finishUserCommandRemoveSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo)
{
	_userCommand.cardState = CardState::InBay;
}

void UserCommandRunner::finishUserCommand(CommandState consoleCmdState, const std::string& errorInfo)
{
	CommandState userCmdResult;
	std::string error = errorInfo;

	if(consoleCmdState != CommandState::Succeeded)
	{
		//error in console command sequence, fail the user command directly.
		userCmdResult = CommandState::Failed;
	}
	else
	{
		userCmdResult = CommandState::Succeeded;

		//check result of console commands to know if user command is fulfilled.
		if(_userCommand.command == UserCmdConnectDevice)
		{
			finishUserCommandConnectDevice(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdCheckResetPressed)
		{
			finishUserCommandCheckResetPressed(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdCheckResetReleased)
		{
			finishUserCommandCheckResetReleased(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdResetDevice)
		{
			finishUserCommandResetDevice(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdInsertSmartCard)
		{
			finishUserCommandInsertSmartCard(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdRemoveSmartCard)
		{
			finishUserCommandRemoveSmartCard(userCmdResult, error);
		}
		else if(_userCommand.command == UserCmdCardFromBayToSmartCardGate)
		{
			_userCommand.cardState = CardState::InSmartCardGate;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToSmartCardReaderGate)
		{
			_userCommand.cardState = CardState::InSmartCardReaderGate;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardReader)
		{
			_userCommand.cardState = CardState::InSmartCardReader;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderToSmartCardReaderGate)
		{
			_userCommand.cardState = CardState::InSmartCardReaderGate;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardGate)
		{
			_userCommand.cardState = CardState::InSmartCardGate;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToBarcodeReaderGate)
		{
			_userCommand.cardState = CardState::InBarcodeReaderGate;
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToBarcodeReader)
		{
			_userCommand.cardState = CardState::InBarcodeReader;
		}
		else if(_userCommand.command == UserCmdCardBarcodeToExtraPosition)
		{
			_userCommand.cardState = CardState::InBarcodeReader;
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderToBarcodeReaderGate)
		{
			_userCommand.cardState = CardState::InBarcodeReaderGate;
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToSmartCardGate)
		{
			_userCommand.cardState = CardState::InSmartCardGate;
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToBay)
		{
			_userCommand.cardState = CardState::InBay;
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromBayToPosition)
		{
			_userCommand.mobileBarcodeState = MobileBarcodeState::InMobileBarcodePosition;
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToPosition)
		{
			_userCommand.mobileBarcodeState = MobileBarcodeState::InMobileBarcodePosition;
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToBay)
		{
			_userCommand.mobileBarcodeState = MobileBarcodeState::InMobileBarcodeBay;
		}
	}

	//update user command firstly, then notify observers.
	//In this way, new user command can be accepted if it is sent out in the the notification, though sending out new user command in notification is not recommended.
	if(userCmdResult == CommandState::Succeeded) {
		_userCommand.state = CommandState::Idle;
	}
	else if(userCmdResult == CommandState::Failed) {
		_userCommand.state = CommandState::Idle;
	}
	else {
		pLogger->LogError("UserCommandRunner::finishUserCommand wrong user command state: " + std::to_string((int)userCmdResult));
		_userCommand.state = CommandState::Failed;
	}

	notifyObservers(_userCommand.commandId, userCmdResult, error);

	pLogger->LogInfo("UserCommandRunner::finishUserCommand ====== finished user command: " + _userCommand.command);
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

void UserCommandRunner::executeUserCmdConnectDevice()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdDevicesGet();
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdDeviceConnect(0);//only one device at the moment.
	runConsoleCommand(cmd);
	//queries
	cmd = ConsoleCommandFactory::CmdDeviceQueryPower();//device power
	runConsoleCommand(cmd);
}

void UserCommandRunner::parseUserCmdCheckResetPressed(Poco::DynamicStruct& ds)
{
	unsigned int locatorIndex = ds["locatorIndex"];
	unsigned int lineNumber = ds["lineNumber"];

	_userCommand.locatorIndexReset = locatorIndex;
	_userCommand.lineNumberReset = lineNumber;
}

void UserCommandRunner::executeUserCmdCheckResetPressed()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdLocatorQuery(_userCommand.locatorIndexReset);
	runConsoleCommand(cmd);
}

void UserCommandRunner::parseUserCmdCheckResetReleased(Poco::DynamicStruct& ds)
{
	unsigned int locatorIndex = ds["locatorIndex"];
	unsigned int lineNumber = ds["lineNumber"];

	_userCommand.locatorIndexReset = locatorIndex;
	_userCommand.lineNumberReset = lineNumber;
}

void UserCommandRunner::executeUserCmdCheckResetReleased()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdLocatorQuery(_userCommand.locatorIndexReset);
	runConsoleCommand(cmd);
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
											unsigned int decelerationBufferIncrement)
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdStepperConfigStep(index, lowClks, highClks);
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperAccelerationBuffer(index, accelerationBuffer);
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperAccelerationBufferDecrement(index, accelerationBufferDecrement);
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperDecelerationBuffer(index, decelerationBuffer);
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperDecelerationBufferIncrement(index, decelerationBufferIncrement);
	runConsoleCommand(cmd);
}

void UserCommandRunner::executeUserCmdResetDevice()
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
	bool forwardClockwise;

	const unsigned int x = 0;
	const unsigned int y = 1;
	const unsigned int z = 2;
	const unsigned int w = 3;
	const unsigned int v = 4;

	//power on steppers
	cmd = ConsoleCommandFactory::CmdSteppersPowerOn();
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdSteppersQueryPower();
	runConsoleCommand(cmd);
	//power on BDCs
	cmd = ConsoleCommandFactory::CmdBdcsPowerOn();
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdBdcsQueryPower();
	runConsoleCommand(cmd);

	//reset steppers
	unsigned int stepperIndexes[3] = {z, y, x};
	for(unsigned int i=0; i<3; i++)
	{
		auto rc = pMovementConfiguration->GetStepperGoHome(lowClks,
															highClks,
															accelerationBuffer,
															accelerationBufferDecrement,
															decelerationBuffer,
															decelerationBufferIncrement);

		rc = rc && pMovementConfiguration->GetStepperBoundary(stepperIndexes[i],
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);

		rc = rc && pMovementConfiguration->GetStepperForwardClockwise(stepperIndexes[i], forwardClockwise);

		if(rc)
		{
			configStepperMovement(stepperIndexes[i],
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement);

			cmd = ConsoleCommandFactory::CmdStepperForwardClockwise(stepperIndexes[i], forwardClockwise);
			runConsoleCommand(cmd);
			cmd = ConsoleCommandFactory::CmdStepperConfigHome(stepperIndexes[i], locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			runConsoleCommand(cmd);

			cmd = ConsoleCommandFactory::CmdStepperRun(stepperIndexes[i], 0, 0);
			runConsoleCommand(cmd);
		}
		else
		{
			throwError("UserCommandRunner::executeUserCmdResetDevice failed to retrieve configuration for stepper: " + std::to_string(stepperIndexes[i]));
		}
	}
	//set W and V to known position directly
	{
		cmd = ConsoleCommandFactory::CmdStepperSetKnownState(w);
		runConsoleCommand(cmd);

		cmd = ConsoleCommandFactory::CmdStepperSetKnownState(v);
		runConsoleCommand(cmd);
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
			configStepperMovement(i,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement);
		}
		else
		{
			throwError("UserCommandRunner::executeUserCmdResetDevice failed to retrieve general configuration for stepper: " + std::to_string(i));
		}
	}

	//query steppers
	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		cmd = ConsoleCommandFactory::CmdStepperQuery(i);
		runConsoleCommand(cmd);
	}
}

void UserCommandRunner::parseUserCmdAjustStepperW(Poco::DynamicStruct& ds)
{
	_userCommand.wAdjustment = ds["adjustment"];
}

void UserCommandRunner::parseUserCmdFinishStepperWAdjustment(Poco::DynamicStruct& ds)
{
	//nothing is done here
}

void UserCommandRunner::parseUserCmdAjustStepperU(Poco::DynamicStruct& ds)
{
	_userCommand.uAdjustment = ds["adjustment"];
}

void UserCommandRunner::parseUserCmdFinishStepperUAdjustment(Poco::DynamicStruct& ds)
{
	//nothing is done here
}

void UserCommandRunner::executeUserCmdPullUpSmartCard()
{
	if(_userCommand.cardState != CardState::InBay) {
		throwError("UserCommandRunner::executeUserCmdPullUpSmartCard card is being accessed");
	}

	toSmartCardGate();
	openClamp();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	pushUpSmartCardArm();
	gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	closeClamp();
	pullDownSmartCardArm();
	releaseSmartCardArm();
	smartCard_gate_withCard(_userCommand.smartCardNumber);
}

void UserCommandRunner::executeUserCmdAdjustStepperW()
{
	int curW = currentW();
	int x, y, z, w, u;

	pLogger->LogInfo("UserCommandRunner::executeUserCmdAdjustStepperW adjustment: " + std::to_string(_userCommand.wAdjustment));
	pCoordinateStorage->SetWAdjustment(_userCommand.wAdjustment);
	pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, x, y, z, w, u);

	moveStepperW(curW, w);
}

void UserCommandRunner::executeUserCmdFinishStepperWAdjustment()
{
	pLogger->LogInfo("UserCommandRunner::executeUserCmdFinishStepperWAdjustment stepperW adjustment finished");
	_userCommand.wAdjusted = true;
}

void UserCommandRunner::executeUserCmdAdjustStepperU()
{
	int curU = currentU();
	int x, y, z, w, u;

	pLogger->LogInfo("UserCommandRunner::executeUserCmdAdjustStepperU adjustment: " + std::to_string(_userCommand.uAdjustment));
	pCoordinateStorage->SetUAdjustment(_userCommand.uAdjustment);
	pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, x, y, z, w, u);

	moveStepperU(curU, u);
}
void UserCommandRunner::executeUserCmdFinishStepperUAdjustment()
{
	pLogger->LogInfo("UserCommandRunner::executeUserCmdFinishStepperUAdjustment stepperU adjustment finished");
	_userCommand.uAdjusted = true;
}

void UserCommandRunner::executeUserCmd_Card_from_SmartCardGate_to_SmartCardReaderGate()
{
	if(getCurrentPosition() != ClampPosition::SmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardGateToSmartCardReaderGate not in Position::SmartCardGate");
	}
	if(_userCommand.cardState != CardState::InSmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardGateToSmartCardReaderGate no card in smart card gate");
	}

	toSmartCardReaderGate();
}

void UserCommandRunner::executeUserCmd_Card_from_SmartCardReaderGate_to_SmartCardReader()
{
	if(getCurrentPosition() != ClampPosition::SmartCardReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardReaderGateToSmartCardReader not in Position::SmartCardReaderGate");
	}
	if(_userCommand.cardState != CardState::InSmartCardReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardReaderGateToSmartCardReader no card in smart card reader gate");
	}

	gate_smartCardReader_withCard();
}

void UserCommandRunner::executeUserCmd_Card_from_SmartCardReader_to_SmartCardReaderGate()
{
	if(_userCommand.cardState != CardState::InSmartCardReader) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardReaderToSmartCardReaderGate no card in smart card reader");
	}

	smartCardReader_gate_withCard();
}

void UserCommandRunner::executeUserCmd_Card_from_SmartCardReaderGate_to_SmartCardGate()
{
	if(getCurrentPosition() != ClampPosition::SmartCardReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardReaderGateToSmartCardGate not in Position::SmartCardReaderGate");
	}
	if(_userCommand.cardState != CardState::InSmartCardReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardReaderGateToSmartCardGate no card in smart card reader gate");
	}

	toSmartCardGate();
}

void UserCommandRunner::executeUserCmd_Card_from_SmartCardGate_to_BarcodeReaderGate()
{
	if(getCurrentPosition() != ClampPosition::SmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardGateToBarcodeReaderGate not in Position::SmartCardReaderGate");
	}
	if(_userCommand.cardState != CardState::InSmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromSmartCardGateToBarcodeReaderGate no card in smart card reader gate");
	}

	toBarcodeReaderGate();
}

void UserCommandRunner::executeUserCmd_Card_from_BarcodeReaderGate_to_BarcodeReader()
{
	if(getCurrentPosition() != ClampPosition::BarCodeReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToBarcodeReader not in Position::SmartCardReaderGate");
	}
	if(_userCommand.cardState != CardState::InBarcodeReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToBarcodeReader no card in smart card reader gate");
	}

	gate_barcodeReader();;
}

void UserCommandRunner::executeUserCmd_Card_barcode_to_extraPosition()
{
	if(_userCommand.cardState != CardState::InBarcodeReader) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToBarcodeReader no card in smart card reader gate");
	}

	barcodeReader_extraPosition();
}

void UserCommandRunner::executeUserCmd_Card_from_BarcodeReader_to_BarcodeReaderGate()
{
	if(_userCommand.cardState != CardState::InBarcodeReader) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToBarcodeReader no card in smart card reader gate");
	}

	barcodeReader_gate();
}

void UserCommandRunner::executeUserCmd_Card_from_BarcodeReaderGate_to_SmartCardGate()
{
	if(getCurrentPosition() != ClampPosition::BarCodeReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToSmartCardGate not in Position::SmartCardReaderGate");
	}
	if(_userCommand.cardState != CardState::InBarcodeReaderGate) {
		throwError("UserCommandRunner::executeUserCmdCardFromBarcodeReaderGateToSmartCardGate no card in smart card reader gate");
	}

	toSmartCardGate();
}

void UserCommandRunner::executeUserCmdPutBackSmartCard()
{
	if(getCurrentPosition() != ClampPosition::SmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdPutBackSmartCard not in Position::SmartCardGate");
	}
	if(_userCommand.cardState != CardState::InSmartCardGate) {
		throwError("UserCommandRunner::executeUserCmdPutBackSmartCard no card in smart card gate");
	}

	gate_smartCard_withCard(_userCommand.smartCardNumber);
	openClamp();
	smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	releaseClamp();
}

void UserCommandRunner::parseUserCmdSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		throwError("UserCommandRunner::parseUserCmdSmartCard smart card number of range: " + std::to_string(number));
	}

	_userCommand.smartCardNumber = number;
}

void UserCommandRunner::parseUserCmdBarcodeToExtraPosition(Poco::DynamicStruct& ds)
{
	unsigned int positionIndex = ds["positionIndex"];
	if(positionIndex >= pCoordinateStorage->BarcodeReaderExtraPositionsAmount())
	{
		throwError("UserCommandRunner::parseUserCmdBarcodeToExtraPosition position amount of range: " + std::to_string(positionIndex));
	}

	_userCommand.barcodeExtraPositionIndex = positionIndex;
}

void UserCommandRunner::parseUserCmdSwipeSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		throwError("UserCommandRunner::parseUserCmdSwipeSmartCard smart card number of range: " + std::to_string(number));
	}

	_userCommand.smartCardNumber = number;
	_userCommand.downPeriod = ds["downPeriod"];
}

void UserCommandRunner::parseUserCmdTapSmartCard(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		throwError("UserCommandRunner::parseUserCmdTapSmartCard smart card number of range: " + std::to_string(number));
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

int UserCommandRunner::currentV()
{
	if(_consoleCommand.resultSteppers[4].state == StepperState::Unknown) {
		return -1;
	}

	return _consoleCommand.resultSteppers[4].homeOffset;
}

UserCommandRunner::ClampPosition UserCommandRunner::getPosition(int x, int y, int z, int w)
{
	int gateX, gateY, gateZ, gateW, gateU;

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::Home, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::Home;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::SmartCardGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::PedKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::SoftKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::AssistKeyGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::TouchScreenGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::SmartCardReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::BarCodeReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::ContactlessReaderGate;
	}

	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, gateX, gateY, gateZ, gateW);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) {
		return ClampPosition::SmartCardGate;
	}

	pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, gateX, gateY, gateZ, gateW, gateU);
	if((x == gateX) && (y == gateY) && (z == gateZ) && (w == gateW)) { //ignore gateU.
		return ClampPosition::MobileBarcodeGate;
	}

	throw Poco::Exception("UserCommandRunner::getPosition unknown position");
}

UserCommandRunner::ClampPosition UserCommandRunner::getCurrentPosition()
{
	int x, y, z, w;

	x = currentX();
	y = currentY();
	z = currentZ();
	w = currentW();

	auto gate = getPosition(x, y, z, w);
	return gate;
}

void UserCommandRunner::moveSmartCardCarriage(unsigned int cardNumber)
{
	int curV, finalV;

	auto rc = pCoordinateStorage->GetSmartCardOffset(cardNumber, finalV);
	if(rc == false)
	{
		throwError("UserCommandRunner::moveToSmartCard failed to retrieve smart card offset: " + std::to_string(cardNumber));
	}

	curV = currentV();
	moveStepperV(curV, finalV);
}

void UserCommandRunner::pushUpSmartCardArm()
{
	std::string cmd;
	//to be made configurable
	cmd = ConsoleCommandFactory::CmdBdcReverse(1, 3, 2, 3000);
	runConsoleCommand(cmd);
}

void UserCommandRunner::pullDownSmartCardArm()
{
	std::string cmd;
	//to be made configurable
	cmd = ConsoleCommandFactory::CmdBdcForward(1, 3, 2, 2000);
	runConsoleCommand(cmd);
}

void UserCommandRunner::releaseSmartCardArm()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdBdcCoast(1);
	runConsoleCommand(cmd);
}

void UserCommandRunner::moveStepper(unsigned int index, unsigned int initialPos, unsigned int finalPos)
{
	if(index >= STEPPER_AMOUNT) {
		throwError("UserCommandRunner::moveStepper stepper index out of range: " + std::to_string(index));
	}
	if(_consoleCommand.resultSteppers[index].homeOffset != initialPos)
	{
		char buf[256];

		sprintf(buf, "UserCommandRunner::moveStepper wrong initialPos: %d in stepper %d, should be %d", initialPos, index, _consoleCommand.resultSteppers[index].homeOffset);
		throwError(std::string(buf));
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
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperSteps(index, steps);
	runConsoleCommand(cmd);
	cmd = ConsoleCommandFactory::CmdStepperRun(index, initialPos, finalPos);
	runConsoleCommand(cmd);
}

void UserCommandRunner::gateToGate(unsigned int fromX, unsigned int fromY, unsigned int fromZ, unsigned int fromW,
				unsigned int toX, unsigned int toY, unsigned int toZ, unsigned int toW)
{
	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gateToGate ++++ from (%d, %d, %d, %d) to (%d, %d, %d, %d)",
				fromX, fromY, fromZ, fromW, toX, toY, toZ, toW);
		pLogger->LogInfo(buf);
	}

	auto sourceGate = getPosition(fromX, fromY, fromZ, fromW);
	auto targetGate = getPosition(toX, toY, toZ, toW);

	if(sourceGate == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::gateToGate current position is not a gate");
	}
	if(targetGate == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::gateToGate target position is not a gate");
	}

	if(sourceGate == targetGate) {
		return;
	}

	if(sourceGate == ClampPosition::Home)
	{
		switch(targetGate)
		{
			case ClampPosition::Home:
				break; //nothing to be done

			case ClampPosition::SmartCardGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperY(fromY, toY);
				moveStepperW(fromW, toW);
				moveStepperX(fromX, toX);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::SmartCardGate)
	{
		switch(targetGate)
		{
			case ClampPosition::Home:
			{
				int curV = currentV();

				moveStepperV(curV, 0);
				moveStepperX(fromX, toX);
				moveStepperW(fromW, toW);
				moveStepperY(fromY, toY);
				moveStepperZ(fromZ, toZ);
			}
			break;

			case ClampPosition::SmartCardGate:
				break; //nothing to be done

			case ClampPosition::SmartCardReaderGate:
			{
				int tmpW = (fromW + toW)/2;

				moveStepperY(fromY, toY);
				moveStepperW(fromW, tmpW);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
				moveStepperW(tmpW, toW);
			}
			break;

			case ClampPosition::BarCodeReaderGate:
			{
				moveStepperY(fromY, toY);
				moveStepperW(fromW, toW);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
			}
			break;

			case ClampPosition::ContactlessReaderGate:
			{
				moveStepperW(fromW, toW);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
			}
			break;

			case ClampPosition::TouchScreenGate:
			{
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
				moveStepperW(fromW, toW);
			}
			break;

			case ClampPosition::MobileBarcodeGate:
			{
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperW(fromW, toW);
				moveStepperZ(fromZ, toZ);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::SmartCardReaderGate)
	{
		switch(targetGate)
		{
			case ClampPosition::SmartCardGate:
			{
				int tmpW = (fromW + toW)/2;

				moveStepperW(fromW, tmpW);
				moveStepperZ(fromZ, toZ);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperW(tmpW, toW);
			}
			break;

			case ClampPosition::SmartCardReaderGate:
				break; //nothing to be done

			case ClampPosition::TouchScreenGate:
			{
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
				moveStepperW(fromW, toW);
			}
			break;

			case ClampPosition::MobileBarcodeGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperW(fromW, toW);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::BarCodeReaderGate)
	{
		switch(targetGate)
		{
			case ClampPosition::SmartCardGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperX(fromX, toX);
				moveStepperY(fromY, toY);
				moveStepperW(fromW, toW);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::ContactlessReaderGate)
	{
		switch(targetGate)
		{
			case ClampPosition::SmartCardGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperX(fromX, toX);
				moveStepperY(fromY, toY);
				moveStepperW(fromW, toW);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::TouchScreenGate)
	{
		switch(targetGate)
		{
			case ClampPosition::SmartCardReaderGate:
			{
				moveStepperW(fromW, toW);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
			}
			break;

			case ClampPosition::SmartCardGate:
			{
				moveStepperW(fromW, toW);
				moveStepperZ(fromZ, toZ);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
			}
			break;

			case ClampPosition::MobileBarcodeGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperW(fromW, toW);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else if(sourceGate == ClampPosition::MobileBarcodeGate)
	{
		switch(targetGate)
		{
			case ClampPosition::SmartCardGate:
			{
				moveStepperZ(fromZ, toZ);
				moveStepperW(fromW, toW);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
			}
			break;

			case ClampPosition::SmartCardReaderGate:
			{
				moveStepperW(fromW, toW);
				moveStepperY(fromY, toY);
				moveStepperX(fromX, toX);
				moveStepperZ(fromZ, toZ);
			}
			break;

			default:
			{
				throwError("UserCommandRunner::gateToGate target position is not supported");
			}
		}
	}
	else
	{
		throwError("UserCommandRunner::gateToGate current position is not supported");
	}
}

void UserCommandRunner::toHome()
{
	pLogger->LogInfo("UserCommandRunner::toHome ++++");

	auto currentPosition = getCurrentPosition();


	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toHome unknown current position");
	}

	if(currentPosition == ClampPosition::Home) {
		return;
	}

	if(currentPosition != ClampPosition::SmartCardGate) {
		//to smart card gate firstly, then to home, so that carriage runs in safe area.
		toSmartCardGate();
	}

	//to home
	{
		int curX, curY, curZ, curW, curV;
		int x, y, z, w, v;

		//move to home position
		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		curV = currentV();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::Home, x, y, z, w);
		v = 0;
		moveStepperV(curV, v);
		moveStepperW(curW, w);
		moveStepperX(curX, x);
		moveStepperY(curY, y);
		moveStepperZ(curZ, z);
		moveStepperV(curV, v);
	}
}

void UserCommandRunner::toSmartCardGate()
{
	pLogger->LogInfo("UserCommandRunner::toSmartCardGate ++++");

	int curX, curY, curZ, curW;
	int x, y, z, w;

	curZ = currentZ();
	curX = currentX();
	curY = currentY();
	curW = currentW();
	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w);
	gateToGate(curX, curY, curZ, curW, x, y, z, w);
}

void UserCommandRunner::toPedKeyGate()
{
	pLogger->LogInfo("UserCommandRunner::toPedKeyGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toPedKeyGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::PedKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toSoftKeyGate()
{
	pLogger->LogInfo("UserCommandRunner::toSoftKeyGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toSoftKeyGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::SoftKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toAssistKeyGate()
{
	pLogger->LogInfo("UserCommandRunner::toAssistKeyGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toAssistKeyGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::AssistKeyGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toTouchScreenGate()
{
	pLogger->LogInfo("UserCommandRunner::toTouchScreenGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toTouchScreenGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::TouchScreenGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toSmartCardReaderGate()
{
	pLogger->LogInfo("UserCommandRunner::toSmartCardReaderGate ++++");

	int curX, curY, curZ, curW;
	int x, y, z, w;

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();
	pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, x, y, z, w);

	gateToGate(curX, curY, curZ, curW, x, y, z, w);
}

void UserCommandRunner::toContactlessReaderGate()
{
	pLogger->LogInfo("UserCommandRunner::toContactlessReaderGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toContactlessReaderGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::ContactlessReaderGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toBarcodeReaderGate()
{
	pLogger->LogInfo("UserCommandRunner::toBarcodeReaderGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toBarcodeReaderGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::BarCodeReaderGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, x, y, z, w);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::toMobileBarcodeGate()
{
	pLogger->LogInfo("UserCommandRunner::toMobileBarcodeGate ++++");

	auto currentPosition = getCurrentPosition();

	if(currentPosition == ClampPosition::Unknown)
	{
		throwError("UserCommandRunner::toMobileBarcodeGate unknown current position");
	}
	if(currentPosition == ClampPosition::Home)
	{
		toSmartCardGate();
	}

	if(currentPosition != ClampPosition::MobileBarcodeGate)
	{
		int curX, curY, curZ, curW;
		int x, y, z, w, u;

		curX = currentX();
		curY = currentY();
		curZ = currentZ();
		curW = currentW();
		pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, x, y, z, w, u);

		gateToGate(curX, curY, curZ, curW, x, y, z, w);
	}
}

void UserCommandRunner::gate_smartCard_withoutCard(unsigned int cardNumber)
{
	std::string cmd;
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long offset;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_smartCard_withoutCard ++++ cardNumber %d", cardNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, curX, curY, curZ, curW);
	if(rc == false)
	{
		throwError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve smart card gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, finalX, finalY, finalZ, finalW, cardNumber);
	if(rc == false)
	{
		throwError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve smart card: " + std::to_string(cardNumber));
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false)
	{
		throwError("UserCommandRunner::gate_smartCard_withoutCard failed to retrieve fetch offset");
	}

	//move to top of smart card
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY - offset);

	//move down
	moveStepperZ(curZ, finalZ);

	moveStepperY(finalY - offset, finalY);
}

void UserCommandRunner::smartCard_gate_withCard(unsigned int cardNumber)
{
	std::string cmd;
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::smartCard_gate_withCard ++++ cardNumber %d", cardNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCard_gate_withCard failed to retrieve smart card gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, curX, curY, curZ, curW, cardNumber);
	if(rc == false) {
		throwError("UserCommandRunner::smartCard_gate_withCard failed to retrieve smart card: " + std::to_string(cardNumber));
	}

	//Z
	moveStepperZ(curZ, finalZ);

	//Y
	moveStepperY(curY, finalY);

	//X
	moveStepperX(curX, finalX);
}

void UserCommandRunner::gate_smartCardReader_withCard()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long slowInsertEnd;
	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;
	long insertExtra;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_smartCardReader_withCard ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader gate");
	}
	rc = pCoordinateStorage->GetSmartCardReaderSlowInsertEndY(slowInsertEnd);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve smart card reader slow insert end");
	}
	rc = pMovementConfiguration->GetStepperCardInsert(lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve stepper card slow insert");
	}
	rc = pCoordinateStorage->GetSmartCardInsertExtra(insertExtra);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve stepper card insert extra");
	}

	moveStepperX(finalX, finalX + 32);

	//configure movement
	configStepperMovement(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	//slow insertion
	moveStepperY(curY, slowInsertEnd);

	//restore to normal speed
	rc = pMovementConfiguration->GetStepperGeneral(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withCard failed to retrieve stepper general");
	}
	//configure movement
	configStepperMovement(STEPPER_Y, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	//insert card
	moveStepperX(finalX + 32, finalX);
	moveStepperY(slowInsertEnd, finalY + insertExtra);
	moveStepperY(finalY + insertExtra, finalY);
}

void UserCommandRunner::smartCardReader_gate_withCard()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::smartCardReader_gate_withCard ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCardReader_gate_withCard failed to retrieve smart card reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCardReader_gate_withCard failed to retrieve smart card reader gate");
	}

	moveStepperY(curY, finalY);
}

void UserCommandRunner::smartCardReader_gate_withoutCard()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long fetchOffset;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::smartCardReader_gate_withoutCard ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve smart card reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve smart card reader gate");
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(fetchOffset);
	if(rc == false) {
		throwError("UserCommandRunner::smartCardReader_gate_withoutCard failed to retrieve fetchOffset");
	}

	moveStepperZ(curZ, curZ - fetchOffset);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ - fetchOffset, curZ);
}

void UserCommandRunner::openClamp()
{
	unsigned long lowClks, highClks, cycles;
	std::vector<std::string> result;
	std::string cmd;

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	cmd = ConsoleCommandFactory::CmdBdcReverse(0, lowClks, highClks, cycles);
	runConsoleCommand(cmd);
}

void UserCommandRunner::closeClamp()
{
	unsigned long lowClks, highClks, cycles;
	std::vector<std::string> result;
	std::string cmd;

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	cmd = ConsoleCommandFactory::CmdBdcForward(0, lowClks, highClks, cycles);
	runConsoleCommand(cmd);
}

void UserCommandRunner::releaseClamp()
{
	std::string cmd;

	cmd = ConsoleCommandFactory::CmdBdcCoast(0);
	runConsoleCommand(cmd);
}

void UserCommandRunner::powerOnOpt(bool on)
{
	std::string cmd;

	if(on) {
		cmd = ConsoleCommandFactory::CmdOptPowerOn();
	}
	else {
		cmd = ConsoleCommandFactory::CmdOptPowerOff();
	}
	runConsoleCommand(cmd);
}

void UserCommandRunner::powerOnDcm(bool on, unsigned int index)
{
	std::string cmd;

	if(on) {
		cmd = ConsoleCommandFactory::CmdDcmPowerOn(index);
	}
	else {
		cmd = ConsoleCommandFactory::CmdDcmPowerOff(index);
	}
	runConsoleCommand(cmd);
}

void UserCommandRunner::executeUserCmdInsertSmartCard()
{
	//check if smart card slot is empty
	if(_userCommand.cardState != CardState::InBay) {
		throwError("UserCommandRunner::executeUserCmdInsertSmartCard card in smart card reader");
	}

	toSmartCardGate();
	openClamp();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	pushUpSmartCardArm();
	gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	closeClamp();
	pullDownSmartCardArm();
	releaseSmartCardArm();
	smartCard_gate_withCard(_userCommand.smartCardNumber);
	toSmartCardReaderGate();
	gate_smartCardReader_withCard();
	openClamp();
	smartCardReader_gate_withoutCard();
	releaseClamp();
}

void UserCommandRunner::gate_smartCardReader_withoutCard()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long offset;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_smartCardReader_withoutCard ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card reader gate");
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCardReader_withoutCard failed to retrieve smart card offset");
	}

	moveStepperZ(curZ, curZ - offset);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ - offset, finalZ);
}

void UserCommandRunner::gate_smartCard_withCard(unsigned int cardNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long slowlyPlaceStart;
	long slowlyPlaceEnd;

	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_smartCard_withCard ++++ cardNumber %d", cardNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, finalX, finalY, finalZ, finalW, _userCommand.smartCardNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card gate");
	}
	rc = pCoordinateStorage->GetSmartCardSlowlyPlaceStartZ(slowlyPlaceStart);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card place start");
	}
	rc = pCoordinateStorage->GetSmartCardSlowlyPlaceEndZ(slowlyPlaceEnd);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve smart card place End");
	}
	rc = pMovementConfiguration->GetStepperCardInsert(lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve stepper card slow insert");
	}

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, slowlyPlaceStart);
	//slow insertion
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	moveStepperZ(slowlyPlaceStart, slowlyPlaceEnd);

	//restore to normal speed
	rc = pMovementConfiguration->GetStepperGeneral(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_smartCard_withCard failed to retrieve stepper card slow insert");
	}
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);

	moveStepperZ(slowlyPlaceEnd, finalZ);
}

void UserCommandRunner::smartCard_gate_withoutCard(unsigned int cardNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;
	long fetchOffset;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::smartCard_gate_withoutCard ++++ cardNumber %d", cardNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, curX, curY, curZ, curW, _userCommand.smartCardNumber);
	if(rc == false) {
		throwError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card gate");
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(fetchOffset);
	if(rc == false) {
		throwError("UserCommandRunner::smartCard_gate_withoutCard failed to retrieve smart card fetch offset");
	}

	//in order not to interfere card in smart card bay
	moveStepperY(curY, curY - fetchOffset);
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY - fetchOffset, finalY);
	moveStepperX(curX, finalX);
}

void UserCommandRunner::executeUserCmdRemoveSmartCard()
{
	//check if smart card slot is empty
	if(_userCommand.cardState != CardState::InSmartCardReader) {
		throwError("UserCommandRunner::expandUserCmdRemoveSmartCard no card in smart card reader");
	}

	toSmartCardReaderGate();
	openClamp();
	gate_smartCardReader_withoutCard();
	closeClamp();
	smartCardReader_gate_withCard();
	toSmartCardGate();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	gate_smartCard_withCard(_userCommand.smartCardNumber);
	openClamp();
	smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	releaseClamp();
}

void UserCommandRunner::executeUserCmdSwipeSmartCard()
{
	//check if smart card slot is empty
	if(_userCommand.cardState != CardState::InBay) {
		throwError("UserCommandRunner::expandUserCmdSwipeSmartCard card in smart card reader");
	}

	toSmartCardGate();
	openClamp();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	pushUpSmartCardArm();
	gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	closeClamp();
	pullDownSmartCardArm();
	releaseSmartCardArm();
	smartCard_gate_withCard(_userCommand.smartCardNumber);
	toSmartCardReaderGate();
	gate_smartCardReader_withCard();
	openClamp();
	closeClamp();
	smartCardReader_gate_withCard();
	toSmartCardGate();
	gate_smartCard_withCard(_userCommand.smartCardNumber);
	openClamp();
	smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	releaseClamp();
}

void UserCommandRunner::deviceDelay(unsigned int clks)
{
	auto cmd = ConsoleCommandFactory::CmdDeviceDelay(clks);
	runConsoleCommand(cmd);
}

void UserCommandRunner::gate_contactlessReader()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_contactlessReader ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_contactlessReader failed to retrieve contactless reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_contactlessReader failed to retrieve contactless reader gate");
	}

	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::contactlessReader_gate()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::contactlessReader_gate ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::contactlessReader_gate failed to retrieve contactless reader gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReader, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::contactlessReader_gate failed to retrieve contactless reader");
	}

	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::executeUserCmdTapSmartCard()
{
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ######");
	toSmartCardGate();
	openClamp();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	pushUpSmartCardArm();
	gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	closeClamp();
	pullDownSmartCardArm();
	releaseSmartCardArm();
	smartCard_gate_withCard(_userCommand.smartCardNumber);
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ###### toContactlessReaderGate");
	toContactlessReaderGate();
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ###### gate_contactlessReader");
	gate_contactlessReader();
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ###### deviceDelay");
	deviceDelay(_userCommand.downPeriod);
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ###### contactlessReader_gate");
	contactlessReader_gate();
	pLogger->LogInfo("UserCommandRunner::executeUserCmdTapSmartCard ###### toSmartCardGate");
	toSmartCardGate();
	gate_smartCard_withCard(_userCommand.smartCardNumber);
	openClamp();
	smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	releaseClamp();
}

void UserCommandRunner::parseUserCmdBarCode(Poco::DynamicStruct& ds)
{
	unsigned int number = ds["smartCardNumber"];
	unsigned int downPeriod = ds["downPeriod"];

	if(number >= pCoordinateStorage->SmartCardsAmount())
	{
		throwError("UserCommandRunner::parseUserCmdShowBarCode bar code card number of range: " + std::to_string(number));
	}

	_userCommand.smartCardNumber = number;
	_userCommand.downPeriod = downPeriod;
}

void UserCommandRunner::parseUserCmdPedKeys(Poco::DynamicStruct& ds)
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
				throwError("UserCommandRunner::parseUserCmdKeys key number out of range: " + std::to_string(number));
			}
		}
		else {
			throwError("UserCommandRunner::parseUserCmdKeys index out of range: " + std::to_string(index));
		}
	}
}

void UserCommandRunner::parseUserCmdSoftKeys(Poco::DynamicStruct& ds)
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
			if(number < pCoordinateStorage->SoftKeysAmount()) {
				_userCommand.keyNumbers[index] = number;
			}
			else {
				throwError("UserCommandRunner::parseUserCmdSoftKeys key number out of range: " + std::to_string(number));
			}
		}
		else {
			throwError("UserCommandRunner::parseUserCmdSoftKeys index out of range: " + std::to_string(index));
		}
	}

}

void UserCommandRunner::parseUserCmdTouchScreenKeys(Poco::DynamicStruct& ds)
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
			if(number < pCoordinateStorage->TouchScreenKeysAmount()) {
				_userCommand.keyNumbers[index] = number;
			}
			else {
				throwError("UserCommandRunner::parseUserCmdTouchScreenKeys key number out of range: " + std::to_string(number));
			}
		}
		else {
			throwError("UserCommandRunner::parseUserCmdTouchScreenKeys index out of range: " + std::to_string(index));
		}
	}

}

void UserCommandRunner::parseUserCmdAdaKeys(Poco::DynamicStruct& ds)
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
			if(number < pCoordinateStorage->AssistKeysAmount()) {
				_userCommand.keyNumbers[index] = number;
			}
			else {
				throwError("UserCommandRunner::parseUserCmdAdaKeys key number of range: " + std::to_string(number));
			}
		}
		else {
			throwError("UserCommandRunner::parseUserCmdAdaKeys index out of range: " + std::to_string(index));
		}
	}
}

void UserCommandRunner::parseUserCmdDcm(Poco::DynamicStruct& ds)
{
	_userCommand.dcmIndex = ds["index"];
	if(_userCommand.dcmIndex >= DCM_AMOUNT) {
		throwError("UserCommandRunner::parseUserCmdDcm dcm index out of range: " + std::to_string(_userCommand.dcmIndex));
	}
}

void UserCommandRunner::parseUserCmdMobileBarcode(Poco::DynamicStruct& ds)
{
	_userCommand.mobileBarcodePositionIndex = ds["positionIndex"];
	if(_userCommand.mobileBarcodePositionIndex >= pCoordinateStorage->MobileBarcodePositionsAmount()) {
		throwError("UserCommandRunner::parseUserCmdMobileBarcode mobile barcode position index out of range: " + std::to_string(_userCommand.mobileBarcodePositionIndex));
	}
}

void UserCommandRunner::barcodeReader_gate()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::barcodeReader_gate ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::barcodeReader_gate failed to retrieve bar code reader");
	}

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);
	moveStepperW(curW, finalW);
}

void UserCommandRunner::barcodeReader_extraPosition()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::barcodeReader_toExtraPosition ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderExtraPosition, finalX, finalY, finalZ, finalW, _userCommand.barcodeExtraPositionIndex);
	if(rc == false) {
		throwError("UserCommandRunner::barcodeReader_gate failed to retrieve bar code reader");
	}
	if((finalX == -1) || (finalY == -1) || (finalZ == -1) || (finalW == -1)) {
		throwError("UserCommandRunner::barcodeReader_extraPosition illegal extra position: " + std::to_string(_userCommand.barcodeExtraPositionIndex));
	}

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();

	if(curZ < finalZ) {
		//move up
		moveStepperX(curX, finalX);
		moveStepperY(curY, finalY);
		moveStepperW(curW, finalW);
		moveStepperZ(curZ, finalZ);
	}
	else {
		//move down
		moveStepperX(curX, finalX);
		moveStepperZ(curZ, finalZ);
		moveStepperW(curW, finalW);
		moveStepperY(curY, finalY);
	}
}

void UserCommandRunner::gate_barcodeReader()
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_barcodeReader ++++ ");
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReader, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_barcodeReader failed to retrieve bar code reader");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_barcodeReader failed to retrieve bar code reader gate");
	}
	moveStepperY(curY, finalY);
}

void UserCommandRunner::executeUserCmdTapBarCode()
{
	toSmartCardGate();
	openClamp();
	moveSmartCardCarriage(_userCommand.smartCardNumber);
	pushUpSmartCardArm();
	gate_smartCard_withoutCard(_userCommand.smartCardNumber);
	closeClamp();
	pullDownSmartCardArm();
	releaseSmartCardArm();
	smartCard_gate_withCard(_userCommand.smartCardNumber);
	toBarcodeReaderGate();
	gate_barcodeReader();
	deviceDelay(_userCommand.downPeriod);
	barcodeReader_gate();
	toSmartCardGate();
	gate_smartCard_withCard(_userCommand.smartCardNumber);
	openClamp();
	smartCard_gate_withoutCard(_userCommand.smartCardNumber);
	releaseClamp();
}

void UserCommandRunner::pullUpKeyPressingArm()
{
	std::string cmd;
	//to be made configurable
	cmd = ConsoleCommandFactory::CmdBdcForward(1, 3, 2, 15000);
	runConsoleCommand(cmd);
}

void UserCommandRunner::putDownKeyPressingArm()
{
	std::string cmd;
	//to be made configurable
	cmd = ConsoleCommandFactory::CmdBdcReverse(1, 3, 2, 15000);
	runConsoleCommand(cmd);
}

void UserCommandRunner::releaseKeyPressingArm()
{
	std::string cmd;

	//to be made configurable
	cmd = ConsoleCommandFactory::CmdBdcCoast(1);
	runConsoleCommand(cmd);
}

void UserCommandRunner::pedKey_gate(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::pedKey_gate ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key:" + std::to_string(keyNumber));
	}

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::pedKey_pedKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::pedKey_pedKey ++++ from %d to %d", keyNumberFrom, keyNumberTo);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key: " + std::to_string(keyNumberTo));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		throwError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key gate");
	}

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key pressed: " + std::to_string(keyNumberTo));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::pedKey_pedKey failed to retrieve ped key: " + std::to_string(keyNumberTo));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::gate_pedKey(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_pedKey ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key: " + std::to_string(keyNumber));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key gate");
	}

	//to key
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key pressed: " + std::to_string(keyNumber));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_pedKey failed to retrieve ped key: " + std::to_string(keyNumber));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}


void UserCommandRunner::executeUserCmdPressPedKey()
{
	if(_userCommand.keyNumbers.empty()) {
		throwError("UserCommandRunner::expandUserCmdPressPedKey no key to press");
	}

	toPedKeyGate();
	putDownKeyPressingArm();

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;

	//press first key
	gate_pedKey(pKeys[0]);

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		pedKey_pedKey(pKeys[i], pKeys[i+1]);
	}

	//back to gate
	pedKey_gate(pKeys[lastKeyIndex]);
	pullUpKeyPressingArm();
	releaseKeyPressingArm();
}

void UserCommandRunner::softKey_gate(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::softKey_gate ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_gate failed to retrieve soft key gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_gate failed to retrieve soft key:" + std::to_string(keyNumber));
	}

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::softKey_softKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::softKey_softKey ++++ from %d to %d", keyNumberFrom, keyNumberTo);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_softKey failed to retrieve soft key: " + std::to_string(keyNumberTo));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_softKey failed to retrieve soft key gate");
	}

	//to key
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_softKey failed to retrieve soft key pressed: " + std::to_string(keyNumberTo));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::softKey_softKey failed to retrieve soft key: " + std::to_string(keyNumberTo));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::gate_softKey(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_softKey ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_softKey failed to retrieve soft key: " + std::to_string(keyNumber));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_softKey failed to retrieve soft key gate");
	}

	//to key
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_softKey failed to retrieve soft key pressed: " + std::to_string(keyNumber));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_softKey failed to retrieve soft key: " + std::to_string(keyNumber));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::executeUserCmdPressSoftKey()
{
	if(_userCommand.keyNumbers.empty()) {
		throwError("UserCommandRunner::expandUserCmdPressSoftKey no key to press");
	}

	toSoftKeyGate();
	putDownKeyPressingArm();

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;

	//press first key
	gate_softKey(pKeys[0]);

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		softKey_softKey(pKeys[i], pKeys[i+1]);
	}

	//back to gate
	softKey_gate(pKeys[lastKeyIndex]);
	pullUpKeyPressingArm();
	releaseKeyPressingArm();
}

void UserCommandRunner::assistKey_gate(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::assistKey_gate ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_gate failed to retrieve assist key gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_gate failed to retrieve assist key:" + std::to_string(keyNumber));
	}

	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::assistKey_assistKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::assistKey_assistKey ++++ from %d to %d", keyNumberFrom, keyNumberTo);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key: " + std::to_string(keyNumberTo));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key gate");
	}

	//to key
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key pressed: " + std::to_string(keyNumberTo));
	}
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::assistKey_assistKey failed to retrieve assist key: " + std::to_string(keyNumberTo));
	}
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::gate_assistKey(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_assistKey ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_assistKey failed to retrieve assist key: " + std::to_string(keyNumber));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_assistKey failed to retrieve assist key gate");
	}

	//to key
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_assistKey failed to retrieve assist key pressed: " + std::to_string(keyNumber));
	}
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_assistKey failed to retrieve assist key: " + std::to_string(keyNumber));
	}
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::executeUserCmdPressAssistKey()
{
	if(_userCommand.keyNumbers.empty()) {
		throwError("UserCommandRunner::expandUserCmdPressAssistKey no key to press");
	}

	toAssistKeyGate();
	//putDownKeyPressingArm();

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;

	//press first key
	gate_assistKey(pKeys[0]);

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		assistKey_assistKey(pKeys[i], pKeys[i+1]);
	}

	//back to gate
	assistKey_gate(pKeys[lastKeyIndex]);
	//pullUpKeyPressingArm();
	//releaseKeyPressingArm();
}

void UserCommandRunner::touchScreenKey_gate(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::touchScreenKey_gate ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, finalX, finalY, finalZ, finalW);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_gate failed to retrieve touch screen key gate");
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, curX, curY, curZ, curW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_gate failed to retrieve touch screen key:" + std::to_string(keyNumber));
	}

	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);
	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::touchScreenKey_touchScreenKey(unsigned int keyNumberFrom, unsigned int keyNumberTo)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::touchScreenKey_touchScreenKey ++++ from %d to %d", keyNumberFrom, keyNumberTo);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumberTo));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, curX, curY, curZ, curW, keyNumberFrom);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key gate");
	}

	//to key
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key pressed: " + std::to_string(keyNumberTo));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumberTo);
	if(rc == false) {
		throwError("UserCommandRunner::touchScreenKey_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumberTo));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}

void UserCommandRunner::gate_touchScreenKey(unsigned int keyNumber)
{
	int curX, curY, curZ, curW;
	int finalX, finalY, finalZ, finalW;

	{
		char buf[256];
		sprintf(buf, "UserCommandRunner::gate_touchScreenKey ++++ keyNumber %d", keyNumber);
		pLogger->LogInfo(buf);
	}

	auto rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumber));
	}
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, curX, curY, curZ, curW);
	if(rc == false) {
		throwError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key gate");
	}

	//to key
	moveStepperZ(curZ, finalZ);
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);

	//press key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key pressed: " + std::to_string(keyNumber));
	}
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperZ(curZ, finalZ);

	//delay
	deviceDelay(_userCommand.downPeriod);

	//release key
	curX = finalX;
	curY = finalY;
	curZ = finalZ;
	rc = pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, finalX, finalY, finalZ, finalW, keyNumber);
	if(rc == false) {
		throwError("UserCommandRunner::gate_touchScreenKey failed to retrieve touch screen key: " + std::to_string(keyNumber));
	}
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);

	//delay
	deviceDelay(_userCommand.upPeriod);
}


void UserCommandRunner::executeUserCmdTouchScreen()
{
	if(_userCommand.keyNumbers.empty()) {
		throwError("UserCommandRunner::expandUserCmdTouchScreen no key to press");
	}

	toTouchScreenGate();

	auto pKeys = _userCommand.keyNumbers.data();
	unsigned int lastKeyIndex = _userCommand.keyNumbers.size() - 1;

	//press first key
	gate_touchScreenKey(pKeys[0]);

	//press other keys
	for(unsigned int i=0; i<lastKeyIndex; i++)
	{
		touchScreenKey_touchScreenKey(pKeys[i], pKeys[i+1]);
	}

	//back to gate
	touchScreenKey_gate(pKeys[lastKeyIndex]);
}

void UserCommandRunner::RunCommand(const std::string& jsonCmd, std::string& errorInfo)
{
	Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex);

	pLogger->LogInfo("UserCommandRunner::RunCommand parse command: ====== " + jsonCmd);

	//pCoordinateStorage->ReloadCoordinate();

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
		else if(_userCommand.command == UserCmdCardFromBayToSmartCardGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdAdjustStepperW) {
			parseUserCmdAjustStepperW(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToBay) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdFinishStepperWAdjustment) {
			parseUserCmdFinishStepperWAdjustment(ds);
		}
		else if(_userCommand.command == UserCmdAdjustStepperU) {
			parseUserCmdAjustStepperU(ds);
		}
		else if(_userCommand.command == UserCmdFinishStepperUAdjustment) {
			parseUserCmdFinishStepperUAdjustment(ds);
		}
		//stop other command if stepper w hasn't been adjusted.
		else if(_userCommand.wAdjusted == false) {
			errorInfo = ErrorStepperWNotAdjusted;
			pLogger->LogError("UserCommandRunner::RunCommand " + errorInfo);
			return;
		}
		//stop other command if stepper u hasn't been adjusted.
		else if(_userCommand.wAdjusted == false) {
			errorInfo = ErrorStepperUNotAdjusted;
			pLogger->LogError("UserCommandRunner::RunCommand " + errorInfo);
			return;
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
		else if(_userCommand.command == UserCmdTapBarCode) {
			parseUserCmdBarCode(ds);
		}
		else if(_userCommand.command == UserCmdPressPedKey) {
			parseUserCmdPedKeys(ds);
		}
		else if(_userCommand.command == UserCmdPressSoftKey) {
			parseUserCmdSoftKeys(ds);
		}
		else if(_userCommand.command == UserCmdPressAssistKey) {
			parseUserCmdAdaKeys(ds);
		}
		else if(_userCommand.command == UserCmdTouchScreen) {
			parseUserCmdTouchScreenKeys(ds);
		}
		else if(_userCommand.command == UserCmdBackToHome)
		{
			if(_userCommand.cardState != CardState::InBay)
			{
				//there is a card on its way, cannot back to home
				errorInfo = ErrorCardIsBeingAccessed;
				pLogger->LogError("UserCommandRunner::RunCommand " + errorInfo);
				return;
			}
		}
		else if(_userCommand.command == UserCmdPowerOnOpt) {
			//no further parameters to parse
		}
		else if(_userCommand.command == UserCmdPowerOffOpt) {
			//no further parameters to parse
		}
		else if(_userCommand.command == UserCmdPowerOnDcm) {
			parseUserCmdDcm(ds);
		}
		else if(_userCommand.command == UserCmdPowerOffDcm) {
			parseUserCmdDcm(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToSmartCardReaderGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardReader) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderToSmartCardReaderGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromSmartCardGateToBarcodeReaderGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToBarcodeReader) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardBarcodeToExtraPosition) {
			parseUserCmdBarcodeToExtraPosition(ds);
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderToBarcodeReaderGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToSmartCardGate) {
			parseUserCmdSmartCard(ds);
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromBayToPosition) {
			parseUserCmdMobileBarcode(ds);
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToPosition) {
			parseUserCmdMobileBarcode(ds);
		}
		else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToBay) {
			;
		}
		else if(_userCommand.command == UserCmdReturnSmartCard) {
			;
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

	_userCommand.state = CommandState::OnGoing;
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

void UserCommandRunner::OnDcmPowerOn(CommandId key, bool bSuccess)
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
		pLogger->LogInfo("UserCommandRunner::OnDcmPowerOn successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDcmPowerOn failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::OnDcmPowerOff(CommandId key, bool bSuccess)
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
		pLogger->LogInfo("UserCommandRunner::OnDcmPowerOff successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnDcmPowerOff failure command Id: " + std::to_string(_consoleCommand.cmdId));
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
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forward = _consoleCommand.stepperForward;
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

		{
			char buffer[256];

			sprintf(buffer, "UserCommandRunner::OnStepperRun offsets: %d, %d, %d, %d, %d",
					_consoleCommand.resultSteppers[0].homeOffset,
					_consoleCommand.resultSteppers[1].homeOffset,
					_consoleCommand.resultSteppers[2].homeOffset,
					_consoleCommand.resultSteppers[3].homeOffset,
					_consoleCommand.resultSteppers[4].homeOffset);

			pLogger->LogInfo(buffer);
		}
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
							bool bForwardClockwise,
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
		stepperData.forwardClockwise = bForwardClockwise;
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

void UserCommandRunner::OnStepperForwardClockwise(CommandId key, bool bSuccess)
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
		pLogger->LogInfo("UserCommandRunner::OnStepperForwardClockwise successful command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forwardClockwise = _consoleCommand.stepperForwardClockwise;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperForwardClockwise failure command Id: " + std::to_string(_consoleCommand.cmdId));
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

void UserCommandRunner::OnStepperSetState(CommandId key, bool bSuccess)
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
		pLogger->LogInfo("UserCommandRunner::OnStepperSetState successful command Id: " + std::to_string(_consoleCommand.cmdId));

		auto& stepperData = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];

		stepperData.state = StepperState::KnownPosition;
		stepperData.homeOffset = 0;
		pLogger->LogInfo("UserCommandRunner::OnStepperSetState set to home position");
		_consoleCommand.state = CommandState::Succeeded;

		{
			char buffer[256];

			sprintf(buffer, "UserCommandRunner::OnStepperRun offsets: %d, %d, %d, %d, %d",
					_consoleCommand.resultSteppers[0].homeOffset,
					_consoleCommand.resultSteppers[1].homeOffset,
					_consoleCommand.resultSteppers[2].homeOffset,
					_consoleCommand.resultSteppers[3].homeOffset,
					_consoleCommand.resultSteppers[4].homeOffset);

			pLogger->LogInfo(buffer);
		}
	}
	else {
		pLogger->LogError("UserCommandRunner::OnStepperSetState failure command Id: " + std::to_string(_consoleCommand.cmdId));
		_consoleCommand.state = CommandState::Failed;
	}
}

void UserCommandRunner::throwError(const std::string& errorInfo)
{
	pLogger->LogError(errorInfo);
	throw Poco::Exception(errorInfo);
}

void UserCommandRunner::setConsoleCommandParameter(const std::string & cmd)
{
	unsigned int stepperIndex, locatorIndex, steps;
	bool stepperForward;

	if(ConsoleCommandFactory::GetParameterStepperIndex(cmd, stepperIndex)) {
		_consoleCommand.stepperIndex = stepperIndex;
	}

	if(ConsoleCommandFactory::GetParameterStepperSteps(cmd, steps)) {
		_consoleCommand.steps = steps;
	}

	if(ConsoleCommandFactory::GetParameterStepperForward(cmd, stepperForward)) {
		_consoleCommand.stepperForward = stepperForward;
	}

	if(ConsoleCommandFactory::GetParameterLocatorIndex(cmd, locatorIndex)) {
		_consoleCommand.locatorIndex = locatorIndex;
	}
}

void UserCommandRunner::runConsoleCommand(const std::string& cmd)
{
	CommandState consoleCmdState;
	std::string cmdToLog;

	//log command content except \r\n
	for(auto it=cmd.begin(); it!=cmd.end(); it++) {
		char c = *it;
		if((c >= ' ') && (c <= '~')) {
			cmdToLog.push_back(c);
		}
	}

	pLogger->LogInfo("UserCommandRunner::runConsoleCommand ------ " + cmdToLog);

	{
		Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

		//check console state
		consoleCmdState = _consoleCommand.state;
		if(consoleCmdState != CommandState::Idle) {
			//throw exception to terminate the current USER command
			throw Poco::Exception("UserCommandRunner::runConsoleCommand wrong console command state: " + std::to_string((int)consoleCmdState));
		}

		//start console command
		_consoleCommand.state = CommandState::OnGoing; //change state here to give a correct state if callback comes instantly.
		setConsoleCommandParameter(cmd);
		_consoleCommand.cmdId = _pConsoleOperator->RunConsoleCommand(cmd);
		pLogger->LogInfo("UserCommandRunner::runConsoleCommand command Id: " + std::to_string(_consoleCommand.cmdId));
		if(_consoleCommand.cmdId == ICommandReception::ICommandDataTypes::InvalidCommandId)
		{
			_consoleCommand.state = CommandState::Idle;

			//failed to run the console command, throw exception to terminate the current USER command
			throwError("UserCommandRunner::runConsoleCommand failed to run: " + cmdToLog);
		}
	}

	//wait for console command result
	for(;;)
	{
		{
			Poco::ScopedLock<Poco::Mutex> consoleLock(_consoleCommandMutex);
			consoleCmdState = _consoleCommand.state;
		}

		if(consoleCmdState != CommandState::OnGoing) {
			break;
		}
		else {
			sleep(10);
		}
	}

	//check console command result
	{
		std::string errorInfo;
		Poco::ScopedLock<Poco::Mutex> lock(_consoleCommandMutex); //lock console cmd mutex

		if(consoleCmdState == CommandState::Succeeded) {
			pLogger->LogInfo("UserCommandRunner::runConsoleCommand succeeded in console command: " + cmdToLog);
		}
		else if(consoleCmdState == CommandState::Failed) {
			errorInfo = "UserCommandRunner::runConsoleCommand failed in console command: " + cmdToLog;
		}
		else {
			errorInfo = "UserCommandRunner::runConsoleCommand wrong console command state: " + std::to_string((int)consoleCmdState);
		}

		_consoleCommand.state = CommandState::Idle;
		if(!errorInfo.empty())
		{
			pLogger->LogError(errorInfo);
			//throw exception to terminate the current USER command
			throw Poco::Exception(errorInfo);
		}
	}
}

void UserCommandRunner::gate_bay_withoutMobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	long offset;
	bool rc;

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, curX, curY, curZ, curW, curU);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withoutMobileBarcode failed to retrieve mobile barcode gate's coordinates");
	}
	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeBay, finalX, finalY, finalZ, finalW, finalU);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withoutMobileBarcode failed to retrieve mobile barcode bay's coordinates");
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false)
	{
		throwError("UserCommandRunner::gate_bay_withoutMobileBarcode failed to retrieve fetch offset");
	}

	moveStepperX(curX, finalX);
	moveStepperU(curU, finalU);
	moveStepperW(curW, finalW);
	moveStepperY(curY, finalY - offset);
	openClamp();
	moveStepperZ(curZ, finalZ);
	moveStepperY(finalY - offset, finalY);
	closeClamp();
}

void UserCommandRunner::bay_gate_withMobileBarCode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	bool rc;

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeBay, curX, curY, curZ, curW, curU);
	if(rc == false) {
		throwError("UserCommandRunner::bay_gate_withMobileBarCode failed to retrieve mobile barcode bay's coordinates");
	}
	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, finalX, finalY, finalZ, finalW, finalU);
	if(rc == false) {
		throwError("UserCommandRunner::bay_gate_withMobileBarCode failed to retrieve mobile barcode gate's coordinates");
	}

	moveStepperZ(curZ, finalZ);
	moveStepperY(curY, finalY);
	moveStepperX(curX, finalX);
	moveStepperW(curW, finalW);
	moveStepperU(curU, finalU);
}

void UserCommandRunner::gate_position_mobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	bool rc;
	auto index = _userCommand.mobileBarcodePositionIndex;

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, curX, curY, curZ, curW, curU);
	if(rc == false) {
		throwError("UserCommandRunner::gate_position_mobileBarcode failed to retrieve mobile barcode gate's coordinates");
	}
	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodePosition, finalX, finalY, finalZ, finalW, finalU, index);
	if(rc == false) {
		throwError("UserCommandRunner::gate_position_mobileBarcode failed to retrieve mobile barcode position's coordinates: " + std::to_string(index));
	}

	moveStepperY(curY, finalY);
	moveStepperW(curW, finalW);
	moveStepperX(curX, finalX);
	moveStepperU(curU, finalU);
	moveStepperZ(curZ, finalZ);
}

void UserCommandRunner::position_position_mobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	bool rc;
	auto index = _userCommand.mobileBarcodePositionIndex;

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();
	curU = currentU();

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodePosition, finalX, finalY, finalZ, finalW, finalU, index);
	if(rc == false) {
		throwError("UserCommandRunner::position_position_mobileBarcode failed to retrieve mobile barcode position's coordinates: " + std::to_string(index));
	}

	if(finalZ < curZ) {
		//move down
		moveStepperY(curY, finalY);
		moveStepperW(curW, finalW);
		moveStepperX(curX, finalX);
		moveStepperU(curU, finalU);
		moveStepperZ(curZ, finalZ); //move down lastly
	}
	else {
		//move up
		moveStepperZ(curZ, finalZ); //move up firstly
		moveStepperY(curY, finalY);
		moveStepperW(curW, finalW);
		moveStepperX(curX, finalX);
		moveStepperU(curU, finalU);
	}
}

void UserCommandRunner::position_gate_mobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	bool rc;

	curX = currentX();
	curY = currentY();
	curZ = currentZ();
	curW = currentW();
	curU = currentU();

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, finalX, finalY, finalZ, finalW, finalU);
	if(rc == false) {
		throwError("UserCommandRunner::position_gate_mobileBarcode failed to retrieve mobile barcode gate's coordinates");
	}

	moveStepperZ(curZ, finalZ);
	moveStepperW(curW, finalW);
	moveStepperU(curU, finalU);
	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
}

void UserCommandRunner::gate_bay_withMobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	long offset;
	bool rc;
	long lowClks;
	long highClks;
	long accelerationBuffer;
	long accelerationBufferDecrement;
	long decelerationBuffer;
	long decelerationBufferIncrement;


	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, curX, curY, curZ, curW, curU);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withMobileBarcode failed to retrieve mobile barcode gate's coordinates");
	}
	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeBay, finalX, finalY, finalZ, finalW, finalU);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withMobileBarcode failed to retrieve mobile barcode bay's coordinates");
	}
	rc = pCoordinateStorage->GetMobileBarcodeSlowlyPlaceEnd(offset);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withMobileBarcode failed to retrieve place end offset");
	}
	rc = pMovementConfiguration->GetStepperCardInsert(lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withMobileBarcode failed to retrieve stepper card slow insert");
	}

	moveStepperX(curX, finalX);
	moveStepperY(curY, finalY);
	moveStepperW(curW, finalW);
	moveStepperU(curU, finalU);

	//slow insertion
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	moveStepperZ(curZ, curZ - offset);

	//restore to normal speed
	rc = pMovementConfiguration->GetStepperGeneral(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	if(rc == false) {
		throwError("UserCommandRunner::gate_bay_withMobileBarcode failed to retrieve stepper card slow insert");
	}
	configStepperMovement(STEPPER_Z, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	moveStepperZ(curZ - offset, finalZ);
}

void UserCommandRunner::bay_gate_withoutMobileBarcode()
{
	int curX, curY, curZ, curW, curU;
	int finalX, finalY, finalZ, finalW, finalU;
	long offset;
	bool rc;

	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeBay, curX, curY, curZ, curW, curU);
	if(rc == false) {
		throwError("UserCommandRunner::bay_gate_withoutMobileBarcode failed to retrieve mobile barcode bay's coordinates");
	}
	rc = pCoordinateStorage->GetCoordinateEx(CoordinateStorage::Type::MobileBarcodeGate, finalX, finalY, finalZ, finalW, finalU);
	if(rc == false) {
		throwError("UserCommandRunner::bay_gate_withoutMobileBarcode failed to retrieve mobile barcode gate's coordinates");
	}
	rc = pCoordinateStorage->GetSmartCardFetchOffset(offset);
	if(rc == false)
	{
		throwError("UserCommandRunner::bay_gate_withoutMobileBarcode failed to retrieve fetch offset");
	}

	openClamp();
	moveStepperY(curY, curY - offset);
	moveStepperZ(curZ, finalZ);
	moveStepperY(curY - offset, finalY);
	moveStepperW(curW, finalW);
	moveStepperU(curU, finalU);
	releaseClamp();
}

void UserCommandRunner::executeUserCmd_moveMobileBarcode_from_Bay_to_Position()
{
	toMobileBarcodeGate();
	gate_bay_withoutMobileBarcode();
	bay_gate_withMobileBarCode();
	gate_position_mobileBarcode();
}

void UserCommandRunner::executeUserCmd_moveMobileBarcode_from_Position_to_Position()
{
	position_position_mobileBarcode();
}

void UserCommandRunner::executeUserCmd_moveMobileBarcode_from_Position_to_Bay()
{
	position_gate_mobileBarcode();
	gate_bay_withMobileBarcode();
	bay_gate_withoutMobileBarcode();
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

			{
				Poco::ScopedLock<Poco::Mutex> lock(_userCommandMutex); //lock user cmd mutex
				userCmdState = _userCommand.state;
			}
			if(userCmdState == CommandState::Failed)
			{
				//user command failed
				pLogger->LogError("UserCommandRunner::runTask user command failed, device needs to be reset");
				sleep(1000);
				continue;
			}
			else if(userCmdState == CommandState::Idle)
			{
				// no user command is on-going
				sleep(10);
				continue;
			}
			else if(userCmdState != CommandState::OnGoing)
			{
				pLogger->LogError("UserCommandRunner::runTask wrong user command state: " + std::to_string((int)userCmdState));
				sleep(1000);
				continue;
			}

			//run user command
			std::string errorInfo;
			try
			{
				//expand USER command to CONSOLE commands
				if(_userCommand.command == UserCmdConnectDevice) {
					executeUserCmdConnectDevice();
				}
				else if(_userCommand.command == UserCmdCheckResetPressed) {
					executeUserCmdCheckResetPressed();
				}
				else if(_userCommand.command == UserCmdCheckResetReleased) {
					executeUserCmdCheckResetReleased();
				}
				else if(_userCommand.command == UserCmdResetDevice) {
					executeUserCmdResetDevice();
				}
				else if(_userCommand.command == UserCmdAdjustStepperW) {
					executeUserCmdAdjustStepperW();
				}
				else if(_userCommand.command == UserCmdFinishStepperWAdjustment) {
					executeUserCmdFinishStepperWAdjustment();
				}
				else if(_userCommand.command == UserCmdAdjustStepperU) {
					executeUserCmdAdjustStepperU();
				}
				else if(_userCommand.command == UserCmdFinishStepperUAdjustment) {
					executeUserCmdFinishStepperUAdjustment();
				}
				else if(_userCommand.command == UserCmdInsertSmartCard)
				{
					if(_userCommand.cardState != CardState::InBay) {
						errorInfo = ErrorSmartCardReaderSlotOccupied;
					}
					else {
						executeUserCmdInsertSmartCard();
					}
				}
				else if(_userCommand.command == UserCmdRemoveSmartCard)
				{
					if(_userCommand.cardState != CardState::InSmartCardReader) {
						errorInfo = ErrorSmartCardReaderEmpty;
					}
					else {
						executeUserCmdRemoveSmartCard();
					}
				}
				else if(_userCommand.command == UserCmdSwipeSmartCard)
				{
					if(_userCommand.cardState != CardState::InBay) {
						errorInfo = ErrorSmartCardReaderSlotOccupied;
					}
					else {
						executeUserCmdSwipeSmartCard();
					}
				}
				else if(_userCommand.command == UserCmdTapSmartCard) {
					executeUserCmdTapSmartCard();
				}
				else if(_userCommand.command == UserCmdTapBarCode) {
					executeUserCmdTapBarCode();
				}
				else if(_userCommand.command == UserCmdPressPedKey) {
					executeUserCmdPressPedKey();
				}
				else if(_userCommand.command == UserCmdPressSoftKey) {
					executeUserCmdPressSoftKey();
				}
				else if(_userCommand.command == UserCmdPressAssistKey) {
					executeUserCmdPressAssistKey();
				}
				else if(_userCommand.command == UserCmdTouchScreen) {
					executeUserCmdTouchScreen();
				}
				else if(_userCommand.command == UserCmdBackToHome) {
					toHome();
				}
				else if(_userCommand.command == UserCmdPowerOnOpt) {
					powerOnOpt(true);
				}
				else if(_userCommand.command == UserCmdPowerOffOpt) {
					powerOnOpt(false);
				}
				else if(_userCommand.command == UserCmdPowerOnDcm) {
					powerOnDcm(true, _userCommand.dcmIndex);
				}
				else if(_userCommand.command == UserCmdPowerOffDcm) {
					powerOnDcm(false, _userCommand.dcmIndex);
				}
				else if(_userCommand.command == UserCmdCardFromBayToSmartCardGate)
				{
					if(_userCommand.cardState != CardState::InBay) {
						errorInfo = ErrorSmartCardHasBeenFetched;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmdPullUpSmartCard();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardGateToSmartCardReaderGate)
				{
					if(_userCommand.cardState != CardState::InSmartCardGate) {
						errorInfo = ErrorSmartCardNotInSmartCardGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_SmartCardGate_to_SmartCardReaderGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardReader)
				{
					if(_userCommand.cardState != CardState::InSmartCardReaderGate) {
						errorInfo = ErrorSmartCardNotInSmartCardReaderGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_SmartCardReaderGate_to_SmartCardReader();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardReaderToSmartCardReaderGate)
				{
					if(_userCommand.cardState != CardState::InSmartCardReader) {
						errorInfo = ErrorSmartCardReaderEmpty;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_SmartCardReader_to_SmartCardReaderGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardReaderGateToSmartCardGate)
				{
					if(_userCommand.cardState != CardState::InSmartCardReaderGate) {
						errorInfo = ErrorSmartCardNotInSmartCardReaderGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_SmartCardReaderGate_to_SmartCardGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardGateToBarcodeReaderGate)
				{
					if(_userCommand.cardState != CardState::InSmartCardGate) {
						errorInfo = ErrorSmartCardNotInSmartCardGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_SmartCardGate_to_BarcodeReaderGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToBarcodeReader)
				{
					if(_userCommand.cardState != CardState::InBarcodeReaderGate) {
						errorInfo = ErrorSmartCardNotInBarcodeReaderGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_BarcodeReaderGate_to_BarcodeReader();
					}
				}
				else if(_userCommand.command == UserCmdCardBarcodeToExtraPosition)
				{
					if(_userCommand.cardState != CardState::InBarcodeReader) {
						errorInfo = ErrorSmartCardNotInBarcodeReader;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_barcode_to_extraPosition();
					}
				}
				else if(_userCommand.command == UserCmdCardFromBarcodeReaderToBarcodeReaderGate)
				{
					if(_userCommand.cardState != CardState::InBarcodeReader) {
						errorInfo = ErrorSmartCardNotInBarcodeReader;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_BarcodeReader_to_BarcodeReaderGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromBarcodeReaderGateToSmartCardGate)
				{
					if(_userCommand.cardState != CardState::InBarcodeReaderGate) {
						errorInfo = ErrorSmartCardNotInBarcodeReaderGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmd_Card_from_BarcodeReaderGate_to_SmartCardGate();
					}
				}
				else if(_userCommand.command == UserCmdCardFromSmartCardGateToBay)
				{
					if(_userCommand.cardState != CardState::InSmartCardGate) {
						errorInfo = ErrorSmartCardNotInSmartCardGate;
						pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
					}
					else {
						executeUserCmdPutBackSmartCard();
					}
				}
				else if(_userCommand.command == UserCmdMobileBarcodeFromBayToPosition)
				{
					if(_userCommand.mobileBarcodeState != MobileBarcodeState::InMobileBarcodeBay) {
						errorInfo = ErrorMobileBarcodeNotInBay;
						pLogger->LogError("UserCommandRunner::runTask mobile barcode state: " + std::to_string((int)_userCommand.mobileBarcodeState));
					}
					else {
						executeUserCmd_moveMobileBarcode_from_Bay_to_Position();
					}
				}
				else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToPosition)
				{
					if(_userCommand.mobileBarcodeState != MobileBarcodeState::InMobileBarcodePosition) {
						errorInfo = ErrorMobileBarcodeNotInBay;
						pLogger->LogError("UserCommandRunner::runTask mobile barcode state: " + std::to_string((int)_userCommand.mobileBarcodeState));
					}
					else {
						executeUserCmd_moveMobileBarcode_from_Position_to_Position();
					}
				}
				else if(_userCommand.command == UserCmdMobileBarcodeFromPositionToBay)
				{
					if(_userCommand.mobileBarcodeState != MobileBarcodeState::InMobileBarcodePosition) {
						errorInfo = ErrorMobileBarcodeNotInBay;
						pLogger->LogError("UserCommandRunner::runTask mobile barcode state: " + std::to_string((int)_userCommand.mobileBarcodeState));
					}
					else {
						executeUserCmd_moveMobileBarcode_from_Position_to_Bay();
					}
				}
				else if(_userCommand.command == UserCmdReturnSmartCard)
				{
					switch(_userCommand.cardState)
					{
						case CardState::InBarcodeReader:
						case CardState::InBarcodeReaderGate:
						case CardState::InSmartCardReader:
						case CardState::InSmartCardReaderGate:
						case CardState::InSmartCardGate:
						case CardState::InBay:
							break; //legal state
						default:
						{
							errorInfo = ErrorSmartCardNotInPredefinedPosition;
							pLogger->LogError("UserCommandRunner::runTask cardState: " + std::to_string((int)_userCommand.cardState));
							break;
						}
					}

					if(errorInfo.empty())
					{
						if(_userCommand.cardState == CardState::InSmartCardReader) {
							executeUserCmd_Card_from_SmartCardReader_to_SmartCardReaderGate();
							_userCommand.cardState = CardState::InSmartCardReaderGate;
						}
						if(_userCommand.cardState == CardState::InSmartCardReaderGate) {
							executeUserCmd_Card_from_SmartCardReaderGate_to_SmartCardGate();
							_userCommand.cardState = CardState::InSmartCardGate;
						}
						if(_userCommand.cardState == CardState::InBarcodeReader) {
							executeUserCmd_Card_from_BarcodeReader_to_BarcodeReaderGate();
							_userCommand.cardState = CardState::InBarcodeReaderGate;
						}
						if(_userCommand.cardState == CardState::InBarcodeReaderGate) {
							executeUserCmd_Card_from_BarcodeReaderGate_to_SmartCardGate();
							_userCommand.cardState = CardState::InSmartCardGate;
						}
						if(_userCommand.cardState == CardState::InSmartCardGate) {
							executeUserCmdPutBackSmartCard();
							_userCommand.cardState = CardState::InBay;
						}
					}
				}
				else {
					errorInfo = "UserCommandRunner::runTask unknown user command: " + _userCommand.command;
					pLogger->LogError(errorInfo);
				}
			}
			catch(Poco::Exception & e)
			{
				_userCommand.state = CommandState::Failed;
				errorInfo = "UserCommandRunner::runTask exception: " + e.displayText();
				pLogger->LogError(errorInfo);
			}
			catch(...)
			{
				_userCommand.state = CommandState::Failed;
				errorInfo = "UserCommandRunner::runTask unknown exception occurred";
				pLogger->LogError(errorInfo);
			}

			if(!errorInfo.empty())
			{
				finishUserCommand(CommandState::Failed, errorInfo);
			}
			else {
				finishUserCommand(CommandState::Succeeded, errorInfo);
			}
		}
	}

	pLogger->LogInfo("UserCommandRunner::runTask exited");
}
