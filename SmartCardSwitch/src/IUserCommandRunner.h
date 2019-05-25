/*
 * IUserCommandRunner.h
 *
 *  Created on: Nov 21, 2018
 *      Author: mikez
 */

#ifndef IUSERCOMMANDRUNNER_H_
#define IUSERCOMMANDRUNNER_H_

#include <string>

class IUserCommandDataType
{
public:
	const std::string UserCmdConnectDevice = "connect device";
	const std::string UserCmdCheckResetPressed = "check reset pressed";
	const std::string UserCmdCheckResetReleased = "check reset released";
	const std::string UserCmdResetDevice = "reset device";
	const std::string UserCmdInsertSmartCard = "insert smart card";
	const std::string UserCmdRemoveSmartCard = "remove smart card";
	const std::string UserCmdSwipeSmartCard = "swipe smart card";
	const std::string UserCmdTapSmartCard = "tap smart card";
	const std::string UserCmdShowBarCode = "show bar code";
	const std::string UserCmdPressPedKey = "press PED key";
	const std::string UserCmdPressSoftKey = "press soft key";
	const std::string UserCmdPressAssistKey = "press assist key";
	const std::string UserCmdTouchScreen = "touch screen";
	const std::string UserCmdBackToHome = "back to home";
	const std::string UserCmdPowerOnOpt = "power on opt";
	const std::string UserCmdPowerOffOpt = "power off opt";
	const std::string UserCmdPowerOnDcm = "power on dcm";
	const std::string UserCmdPowerOffDcm = "power off dcm";

	const std::string ErrorDeviceNotAvailable = "device hans't been connected";
	const std::string ErrorDeviceNotPowered = "device is not powered";
	const std::string ErrorResetIsNotPressed = "reset is not pressed";
	const std::string ErrorResetIsNotReleased = "reset is not released";
	const std::string ErrorDeviceNotHomePositioned = "device hasn't been home positioned";
	const std::string ErrorUserCommandOnGoing = "a user command is running";
	const std::string ErrorInvalidJsonUserCommand = "user command cannot be parsed";
	const std::string ErrorUnSupportedCommand = "command is not supported";
	const std::string ErrorFailedExpandingConnectDevice = "failed in expanding connect device";
	const std::string ErrorFailedExpandingCheckResetPressed = "failed in expanding check reset pressed";
	const std::string ErrorFailedExpandingCheckResetReleased = "failed in expanding check reset released";
	const std::string ErrorSteppersNotPoweredAfterReset = "steppers are not powered after reset";
	const std::string ErrorBdcsNotPoweredAfterReset = "BDCs are not powered after reset";
	const std::string ErrorFailedExpandingResetDevice = "failed in expanding reset device";
	const std::string ErrorSmartCardReaderSlotOccupied = "smart card reader is occupied";
	const std::string ErrorSmartCardReaderEmpty = "no card in smart card reader";
	const std::string ErrorFailedToRunConsoleCommand = "failed to run console command";
	const std::string ErrorFailedToRunUserCommand = "failed to run user command";

	const std::string UserCmdStatusOnGoing = "ongoing";
	const std::string UserCmdStatusFailed = "failed";
	const std::string UserCmdStatusSucceeded = "succeeded";
	const std::string UserCmdStatusInternalError = "internal error";

};

/**
 * ==== Examples of user commands ====
 *
 * {
 * 	"userCommand":"connect device",
 * 	"deviceName":"name of device",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * {
 * 	"userCommand":"check reset pressed",
 * 	"commandId":"uniqueCommandId",
 * 	"locatorIndex":0,
 * 	"lineNumber":0
 * }
 *
 * {
 * 	"userCommand":"check reset released",
 * 	"commandId":"uniqueCommandId",
 * 	"locatorIndex":0,
 * 	"lineNumber":0
 * }
 *
 * {
 * 	"userCommand":"reset device"
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * {
 *	"userCommand":"power on opt",
 *	"commandId":"unqueCommandId"
 * }
 *
 * {
 * 	"userCommand":"power off opt",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * {
 * 	"userCommand":"power on dcm",
 * 	"commandId":"uniqueCommandId",
 * 	"index":0
 * }
 *
 * {
 * 	"userCommand":"power off dcm",
 * 	"commandId":"uniqueCommandId",
 * 	"index":0
 * }
 *
 * {
 * 	"userCommand":"insert smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"remove smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"swipe smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0,
 * 	"downPeriod":1
 * }
 *
 * {
 * 	"userCommand":"tap smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0,
 * 	"downPeriod":1
 * }
 *
 * {
 * 	"userCommand":"show bar code",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0,
 * 	"downPeriod":1
 * }
 *
 * {
 * 	"userCommand":"press PED key",
 * 	"keys":[
 * 		{"index":0, "keyNumber":9},
 * 		{"index":1, "keyNumber":7}
 * 	],
 * 	"downPeriod":1000,
 * 	"commandId":"uniqueCommandId",
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"press soft key",
 * 	"keys":[
 * 		{"index":0, "keyNumber":6},
 * 		{"index":1, "keyNumber":3}
 * 	],
 * 	"downPeriod":1000,
 * 	"commandId":"uniqueCommandId",
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"press assist key",
 * 	"commandId":"uniqueCommandId",
 * 	"keys":[
 * 		{"index":0, "keyNumber":2},
 * 		{"index":1, "keyNumber":9}
 * 	],
 * 	"downPeriod":1000,
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"touch screen",
 * 	"commandId":"uniqueCommandId",
 * 	"keys":[
 * 		{"index":0, "keyNumber":1},
 * 		{"index":1, "keyNumber":5}
 * 	],
 * 	"downPeriod":1000,
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"back to home",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 */

class IUserCommandRunner: public IUserCommandDataType
{
public:
	virtual ~IUserCommandRunner() { }

	// error is empty if JSON command is accepted.
	virtual void RunCommand(const std::string& jsonCmd, std::string& error) = 0;
};


/**
 * ==== Example of command status ====
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"ongoing"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"failed",
 * 	"errorInfo":"reason to failure"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"succeeded"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"internal error"
 * }
 *
 */
class IUserCommandRunnerObserver: public IUserCommandDataType
{
public:
	virtual ~IUserCommandRunnerObserver() {}

	virtual void OnCommandStatus(const std::string& jsonStatus) = 0;
};

#endif /* IUSERCOMMANDRUNNER_H_ */
