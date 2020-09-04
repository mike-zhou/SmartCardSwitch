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
	const std::string UserCmdAdjustStepperW = "adjust stepper w";
	const std::string UserCmdFinishStepperWAdjustment = "finish stepper w adjustment";
	const std::string UserCmdTapSmartCard = "tap smart card";
	const std::string UserCmdPressPedKey = "press PED key";
	const std::string UserCmdPressSoftKey = "press soft key";
	const std::string UserCmdPressAssistKey = "press assist key";
	const std::string UserCmdTouchScreen = "touch screen";
	const std::string UserCmdBackToHome = "back to home";
	const std::string UserCmdPowerOnOpt = "power on opt";
	const std::string UserCmdPowerOffOpt = "power off opt";
	const std::string UserCmdPowerOnDcm = "power on dcm";
	const std::string UserCmdPowerOffDcm = "power off dcm";

	//use either power commands or sub commands to insert, remove, swipe smart card or tap bar code.
	const std::string UserCmdReturnSmartCard = "return smart card";
	//power commands
	const std::string UserCmdInsertSmartCard = "insert smart card";
	const std::string UserCmdRemoveSmartCard = "remove smart card";
	const std::string UserCmdSwipeSmartCard = "swipe smart card";
	const std::string UserCmdTapBarCode = "tap bar code";
	//sub commands
	const std::string UserCmdCardFromBayToSmartCardGate = "move card from bay to smartCardGate";
	const std::string UserCmdCardFromSmartCardGateToSmartCardReaderGate = "move card from smartCardGate to smartCardReaderGate";
	const std::string UserCmdCardFromSmartCardReaderGateToSmartCardReader = "move card from smartCardReaderGate to smartCardReader";
	const std::string UserCmdCardFromSmartCardReaderGateToSmartCardIntermediate = "move card from smartCardReaderGate to smartCardIntermediate";
	const std::string UserCmdCardFromSmartCardIntermediateToSmartCardReader = "move card from smartCardIntermediate to smartCardReader";
	const std::string UserCmdCardFromSmartCardReaderToSmartCardReaderGate = "move card from smartCardReader to smartCardReaderGate";
	const std::string UserCmdCardFromSmartCardReaderToSmartCardIntermediate = "move card from smartCardReader to smartCardIntermediate";
	const std::string UserCmdCardFromSmartCardIntermediateToSmartCardReaderGate = "move card from smartCardIntermediate to smartCardReaderGate";
	const std::string UserCmdCardFromSmartCardReaderGateToSmartCardGate = "move card from smartCardReaderGate to smartCardGate";
	const std::string UserCmdCardFromSmartCardGateToBarcodeReaderGate = "move card from smartCardGate to barcodeReaderGate";
	const std::string UserCmdCardFromBarcodeReaderGateToBarcodeReader = "move card from barcodeReaderGate to barcodeReader";
	const std::string UserCmdCardBarcodeToExtraPosition = "move card barcode to extra position";
	const std::string UserCmdCardFromBarcodeReaderToBarcodeReaderGate = "move card from barcodeReader to barcodeReaderGate";
	const std::string UserCmdCardFromBarcodeReaderGateToSmartCardGate = "move card from barcodeReaderGate to smartCardGate";
	const std::string UserCmdCardFromSmartCardGateToBay = "move card from smartCardGate to bay";

	const std::string ErrorDeviceNotAvailable = "device hans't been connected";
	const std::string ErrorDeviceNotPowered = "device is not powered";
	const std::string ErrorResetIsNotPressed = "reset is not pressed";
	const std::string ErrorResetIsNotReleased = "reset is not released";
	const std::string ErrorDeviceNotHomePositioned = "device hasn't been home positioned";
	const std::string ErrorCardIsBeingAccessed = "a smart card is being accessed";
	const std::string ErrorUserCommandOnGoing = "a user command is running";
	const std::string ErrorInvalidJsonUserCommand = "user command cannot be parsed";
	const std::string ErrorStepperWNotAdjusted = "stepper W hasn't been adjusted";
	const std::string ErrorUnSupportedCommand = "command is not supported";
	const std::string ErrorFailedExpandingConnectDevice = "failed in expanding connect device";
	const std::string ErrorFailedExpandingCheckResetPressed = "failed in expanding check reset pressed";
	const std::string ErrorFailedExpandingCheckResetReleased = "failed in expanding check reset released";
	const std::string ErrorSteppersNotPoweredAfterReset = "steppers are not powered after reset";
	const std::string ErrorBdcsNotPoweredAfterReset = "BDCs are not powered after reset";
	const std::string ErrorFailedExpandingResetDevice = "failed in expanding reset device";
	const std::string ErrorSmartCardReaderSlotOccupied = "smart card reader is occupied";
	const std::string ErrorSmartCardReaderEmpty = "no card in smart card reader";
	const std::string ErrorSmartCardIntermediateEmpty = "no card in smart card intermediate";
	const std::string ErrorFailedToRunConsoleCommand = "failed to run console command";
	const std::string ErrorFailedToRunUserCommand = "failed to run user command";
	const std::string ErrorSmartCardHasBeenFetched = "smart card has been fetched";
	const std::string ErrorSmartCardNotInSmartCardGate = "no card is in smart card gate";
	const std::string ErrorSmartCardNotInSmartCardReaderGate = "no card is in smart card reader gate";
	const std::string ErrorSmartCardNotInSmartCardIntermediate = "no card is in smart card intermediate";
	const std::string ErrorSmartCardNotInBarcodeReaderGate = "no card is in barcode reader gate";
	const std::string ErrorSmartCardNotInBarcodeReader = "no card is in barcode reader";
	const std::string ErrorSmartCardNotInPredefinedPosition = "card is not in predefined position";

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
 * 	"userCommand":"pull up smart card",
 * 	"commandId":uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"adjust stepper w",
 * 	"commandId":uniqueCommandId",
 * 	"adjustment":0
 * }
 *
 * {
 * 	"userCommand":"put back smart card",
 * 	"commandId":uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"finish stepper w adjustment"
 * 	"commandId":uniqueCommandId",
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
 *	{
 *		"userCommand":"move card from bay to smartCardGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardGate to smartCardReaderGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReaderGate to smartCardReader",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReaderGate to smartCardIntermediate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReaderIntermediate to smartCardReader",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReader to smartCardReaderGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReader to smartCardIntermediate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardIntermediate to smartCardReaderGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardReaderGate to smartCardGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardGate to barcodeReaderGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from barcodeReaderGate to barcodeReader",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card barcode to extra position",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *		"positionIndex":0
 *	}
 *
 *	{
 *		"userCommand":"move card from barcodeReader to barcodeReaderGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from barcodeReaderGate to smartCardGate",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
 *
 *	{
 *		"userCommand":"move card from smartCardGate to bay",
 *		"commandId":"uniqueCommandId",
 *		"smartCardNumber":0
 *	}
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
