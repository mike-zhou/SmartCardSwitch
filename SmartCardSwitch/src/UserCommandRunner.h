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

#include "Poco/Task.h"

#include "CoordinateStorage.h"


/**
 * ==== Examples of user commands ====
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
	virtual void OnCommandStatus(const std::string& commandId, const State& state) = 0;
};

class UserCommandRunner: public Poco::Task
{
public:
	// error is empty if JSON command is accepted.
	void RunCommand(const std::string& jsonCmd, std::string& error);

	void GetCommandState(std::string& commandId, IUserCommandRunnerObserver::State& state);

private:
	//Poco::Task
	void runTask();

private:
	const std::string UserCmdInsertSmartCard = "insert smart card";
	const std::string UserCmdRemoveSmartCard = "remove smart card";
	const std::string UserCmdSwipeSmartCard = "swipe smart card";
	const std::string UserCmdTapSmartCard = "tap smart card";
	const std::string UserCmdShowBarCode = "show bar code";
	const std::string UserCmdPressPedKey = "press PED key";
	const std::string UserCmdPressSoftKey = "press soft key";
	const std::string UserCmdAssistKey = "press assist key";
	const std::string UserCmdTouchScreen = "touch screen";

	Poco::Mutex _mutex;

	CoordinateStorage::Type _currentPosition;

	std::string _jsonCommand; //original JSON command
	IUserCommandRunnerObserver::State _state;
	//command details
	struct ExpandedCommand
	{
		std::string command;
		std::string commandId;

		//specific data for smart card related command
		unsigned int smartCardIndex;
		//specific data for bar code releated command
		unsigned int barCodeIndex;
		//specific data for keys
		unsigned int downPeriod;
		unsigned int upPeriod;
		std::vector<unsigned int> keyNumbers;

		//low level commands to fullfill this user command
		std::vector<std::string> lowLevelCommands;
	};
	std::vector<ExpandedCommand> _commands;

	enum class ClampState
	{
		Released = 0,
		Opened,
		Closed
	};
	ClampState _clampState;

	bool _smartCardSlotWithCard;
	bool _deviceHomePositioned;

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
	std::vector<std::string> ToHome();
	std::vector<std::string> ToPedKeyGate();
	std::vector<std::string> ToSoftKeyGate();
	std::vector<std::string> ToAssistKeyGate();
	std::vector<std::string> ToTouchScreenGate();
	std::vector<std::string> ToSmartCardSlotGate();
	std::vector<std::string> ToContactlessReaderGate();
	std::vector<std::string> ToBarcodeCardGate();
	std::vector<std::string> ToBarcodeReaderGate();
};



#endif /* USERCOMMANDRUNNER_H_ */
