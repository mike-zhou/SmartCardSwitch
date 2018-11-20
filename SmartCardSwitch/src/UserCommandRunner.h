/*
 * UserCommandRunner.h
 *
 *  Created on: Nov 19, 2018
 *      Author: mikez
 */

#ifndef USERCOMMANDRUNNER_H_
#define USERCOMMANDRUNNER_H_

#include <string>
#include <vector>
#include <deque>

#include "Poco/Task.h"
#include "Poco/Event.h"
#include "Poco/Dynamic/Var.h"

#include "CoordinateStorage.h"
#include "ICommandReception.h"


/**
 * ==== Examples of user commands ====
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
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"tap smart card",
 * 	"commandId":"uniqueCommandId",
 * 	"smartCardNumber":0
 * }
 *
 * {
 * 	"userCommand":"show bar code",
 * 	"commandId":"uniqueCommandId",
 * 	"barCodeNumber":0
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
 * 	"areas":[
 * 		{"index":0, "keyNumber":1},
 * 		{"index":1, "keyNumber":5}
 * 	],
 * 	"downPeriod":1000,
 * 	"upPeriod":1000
 * }
 *
 * {
 * 	"userCommand":"cancel command",
 * 	"commandId":"uniqueCommandId"
 * }
 *
 * ======== Compound command ====
 * [
 * 	{
 * 		"index"=0,
 * 		"command"={
 * 			"userCommand":"insert smart card",
 * 			"commandId":"uniqueCommandId",
 * 			"smartCardNumber":0
 * 		}
 * 	},
 * 	{
 * 		"index"=1,
 * 		"command"={
 * 			"userCommand":"press PED key",
 * 			"keys":[
 * 				{"index":0, "keyNumber":9},
 * 				{"index":1, "keyNumber":7}
 * 			],
 * 			"downPeriod":1000,
 * 			"commandId":"uniqueCommandId",
 * 			"upPeriod":1000
 * 		}
 * 	},
 * 	{
 * 		"index"=2,
 * 		"command"={
 * 			"userCommand":"remove smart card",
 * 			"commandId":"uniqueCommandId",
 * 			"smartCardNumber":0
 * 		}
 * 	}
 * ]
 *
 * ==== Example of command reply ====
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"ongoing"
 * }
 *
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"failed"
 * }
 * {
 * 	"commandId":"uniqueCommandId",
 * 	"result":"succeeded"
 * }
 *
 */


class IUserCommandRunnerObserver
{
public:
	virtual ~IUserCommandRunnerObserver() {}

	enum class State
	{
		Idle = 0,
		OnGoing,
		Failed,
		Succeeded
	};
	virtual void OnCommandStatus(const std::string& commandId, const State& state, const std::string& errorInfo) = 0;
};

class UserCommandRunner: public Poco::Task
{
public:
	UserCommandRunner();

	// error is empty if JSON command is accepted.
	void RunCommand(const std::string& jsonCmd, std::string& error);
	void AddObserver(IUserCommandRunnerObserver * pObserver);

	void GetState(std::string& commandId, IUserCommandRunnerObserver::State& state, std::string& errorInfo);

private:
	//Poco::Task
	void runTask();

private:
	////////////////////////////////////////
	// user command related data and functions
	////////////////////////////////////////

	const std::string UserCmdInsertSmartCard = "insert smart card";
	const std::string UserCmdRemoveSmartCard = "remove smart card";
	const std::string UserCmdSwipeSmartCard = "swipe smart card";
	const std::string UserCmdTapSmartCard = "tap smart card";
	const std::string UserCmdShowBarCode = "show bar code";
	const std::string UserCmdPressPedKey = "press PED key";
	const std::string UserCmdPressSoftKey = "press soft key";
	const std::string UserCmdPressAssistKey = "press assist key";
	const std::string UserCmdTouchScreen = "touch screen";

	const std::string ErrorDeviceNotHomePositioned = "device hasn't been home positioned";
	const std::string ErrorUserCommandOnGoing = "a user command is running";
	const std::string ErrorInvalidJsonUserCommand = "user command cannot be parsed";
	const std::string ErrorUnSupportedCommand = "command is not supported";
	const std::string ErrorFailedExpandingInsertSmartCard = "failed in expanding insert smart card";
	const std::string ErrorFailedExpandingRemoveSmartCard = "failed in expanding remove smart card";
	const std::string ErrorFailedExpandingSwipeSmartCard = "failed in expanding swipe smart card";
	const std::string ErrorFailedExpandingTapSmartCard = "failed in expanding tap smart card";
	const std::string ErrorFailedExpandingShowBarCode = "failed in expanding show bar code";
	const std::string ErrorFailedExpandingPressPedKey = "failed in expanding press PED key";
	const std::string ErrorFailedExpandingPressSoftKey = "failed in expanding press soft key";
	const std::string ErrorFailedExpandingPressAssistKey = "failed in expanding press assist key";
	const std::string ErrorFailedExpandingTouchScreen = "failed in expanding touch screen";

	Poco::Mutex _userCommandMutex;

	CoordinateStorage::Type _currentPosition;
	std::string _jsonUserCommand; //original JSON command
	IUserCommandRunnerObserver::State _state;
	//command details
	struct ExpandedUserCommand
	{
		std::string command;
		std::string commandId;

		//specific data for smart card related command
		unsigned int smartCardNumber;
		//specific data for bar code related command
		unsigned int barCodeNumber;
		//specific data for keys
		unsigned int downPeriod;
		unsigned int upPeriod;
		std::vector<unsigned int> keyNumbers;

		//console commands to fulfill this user command
		std::deque<std::string> consoleCommands;
	};
	ExpandedUserCommand _userCommand;

	enum class ClampState
	{
		Released = 0,
		Opened,
		Closed
	};
	ClampState _clampState;

	bool _smartCardSlotWithCard;
	bool _deviceHomePositioned;

	void notifyObservers(const std::string& cmdId, IUserCommandRunnerObserver::State state, const std::string& errorInfo);

	//fill _userCommand with information in user command JSON
	void parseUserCmdSmartCard(Poco::DynamicStruct& ds);
	void parseUserCmdBarCode(Poco::DynamicStruct& ds);
	void parseUserCmdKeys(Poco::DynamicStruct& ds);

	//return true if user command can be fulfilled with low level commands
	bool expandUserCmdInsertSmartCard();
	bool expandUserCmdRemoveSmartCard();
	bool expandUserCmdSwipeSmartCard();
	bool expandUserCmdTapSmartCard();
	bool expandUserCmdShowBarCode();
	bool expandUserCmdPressPedKey();
	bool expandUserCmdPressSoftKey();
	bool expandUserCmdPressAssistKey();
	bool expandUserCmdTouchScreen();

	//clamp operation
	std::vector<std::string> openClamp();
	std::vector<std::string> closeClamp();
	std::vector<std::string> releaseCard();
	//movement between smart card and gate
	std::vector<std::string> gate_smartCard(unsigned int cardNumber);
	std::vector<std::string> smartCard_gate(unsigned int cardNumber);
	//movement between PED keys and gate
	std::vector<std::string> pedKey_gate(unsigned int keyNumber);
	std::vector<std::string> pedKey_pedKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	std::vector<std::string> gate_pedKey(unsigned int keyNumber);
	//movement between softkey and gate
	std::vector<std::string> softKey_gate(unsigned int keyNumber);
	std::vector<std::string> softKey_softKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	std::vector<std::string> gate_softKey(unsigned int keyNumber);
	//movement between assist key and gate
	std::vector<std::string> assistKey_gate(unsigned int keyNumber);
	std::vector<std::string> assistKey_assistKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	std::vector<std::string> gate_assistKey(unsigned int keyNumber);
	//movement between touch screen area and gate
	std::vector<std::string> touchScreenKey_gate(unsigned int keyNumber);
	std::vector<std::string> touchScreenKey_touchScreenKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	std::vector<std::string> gate_touchScreenKey(unsigned int keyNumber);
	//movement between smart card slot and gate
	std::vector<std::string> smartCardSlot_gate();
	std::vector<std::string> gate_smartCardSlot();
	//movement between contactless reader and gate
	std::vector<std::string> contactlessReader_gate();
	std::vector<std::string> gate_contactlessReader();
	//movement between barcode reader and gate
	std::vector<std::string> barcodeReader_gate();
	std::vector<std::string> gate_barcodeReader();
	//movement between barcode cards and gate
	std::vector<std::string> barcodeCard_gate(unsigned int cardNumber);
	std::vector<std::string> gate_barcodeCard(unsigned int cardNumber);
	//from Gate to Gate
	std::vector<std::string> toHome();
	std::vector<std::string> toPedKeyGate();
	std::vector<std::string> toSoftKeyGate();
	std::vector<std::string> toAssistKeyGate();
	std::vector<std::string> toTouchScreenGate();
	std::vector<std::string> toSmartCardSlotGate();
	std::vector<std::string> toContactlessReaderGate();
	std::vector<std::string> toBarcodeCardGate();
	std::vector<std::string> toBarcodeReaderGate();

	std::vector<IUserCommandRunnerObserver *> _observerPtrArray;

	//////////////////////////////////////
	// console command data and functions
	//////////////////////////////////////

	Poco::Mutex _consoleCommandMutex;
	Poco::Event _consoleEvent;
	struct ConsoleCommand
	{
		ICommandReception::CommandId cmdId;
		enum class CommandState
		{
			Pending = 0,
			Succeeded,
			Failed
		};
		CommandState state;
	};
	ConsoleCommand _consoleCommand;
};

#endif /* USERCOMMANDRUNNER_H_ */
