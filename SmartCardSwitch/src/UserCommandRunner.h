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
#include "ConsoleCommandFactory.h"
#include "ICommandReception.h"
#include "IUserCommandRunner.h"
#include "ConsoleOperator.h"


/**
 * This class accepts user command with IUserCommandRunner interface,
 * expands user command to consoles commands,
 * passes those console command to ConsoleOperator instance one by one,
 * and sends result of user command to IUserCommandRunnerObserver
 */
class UserCommandRunner: public Poco::Task, public IUserCommandRunner, public IResponseReceiver
{
public:
	UserCommandRunner();

	void AddObserver(IUserCommandRunnerObserver * pObserver);
	void SetConsoleOperator(ConsoleOperator * pCO);

private:
	//Poco::Task
	void runTask();

	//IUserCommandRunner
	virtual void RunCommand(const std::string& jsonCmd, std::string& error) override;


private:
	////////////////////////////////////////
	// user command related data and functions
	////////////////////////////////////////

	const std::string UserCmdConnectDevice = "connect device";
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

	const std::string ErrorDeviceNotAvailable = "device hans't been connected";
	const std::string ErrorDeviceNotHomePositioned = "device hasn't been home positioned";
	const std::string ErrorUserCommandOnGoing = "a user command is running";
	const std::string ErrorInvalidJsonUserCommand = "user command cannot be parsed";
	const std::string ErrorUnSupportedCommand = "command is not supported";
	const std::string ErrorFailedExpandingConnectDevice = "failed in expanding connect device";
	const std::string ErrorFailedExpandingResetDevice = "failed in expanding reset device";
	const std::string ErrorFailedExpandingInsertSmartCard = "failed in expanding insert smart card";
	const std::string ErrorFailedExpandingRemoveSmartCard = "failed in expanding remove smart card";
	const std::string ErrorFailedExpandingSwipeSmartCard = "failed in expanding swipe smart card";
	const std::string ErrorFailedExpandingTapSmartCard = "failed in expanding tap smart card";
	const std::string ErrorFailedExpandingShowBarCode = "failed in expanding show bar code";
	const std::string ErrorFailedExpandingPressPedKey = "failed in expanding press PED key";
	const std::string ErrorFailedExpandingPressSoftKey = "failed in expanding press soft key";
	const std::string ErrorFailedExpandingPressAssistKey = "failed in expanding press assist key";
	const std::string ErrorFailedExpandingTouchScreen = "failed in expanding touch screen";
	const std::string ErrorFailedToRunConsoleCommand = "failed to run console command";
	const std::string ErrorFailedToRunUserCommand = "failed to run user command";

	Poco::Mutex _userCommandMutex;

	CoordinateStorage::Type _currentPosition;
	std::string _jsonUserCommand; //original JSON command

	enum class CommandState
	{
		Idle = 0,
		OnGoing,
		Failed,
		Succeeded
	};

	//command details
	struct ExpandedUserCommand
	{
		CommandState state;
		std::string command;
		std::string commandId;

		//specific data for connect device
		std::string deviceName;
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

	void notifyObservers(const std::string& cmdId, CommandState state, const std::string& errorInfo);

	//fill _userCommand with information in user command JSON
	void parseUserCmdConnectDevice(Poco::DynamicStruct& ds);
	void parseUserCmdResetDevice(Poco::DynamicStruct& ds);
	void parseUserCmdSmartCard(Poco::DynamicStruct& ds);
	void parseUserCmdBarCode(Poco::DynamicStruct& ds);
	void parseUserCmdKeys(Poco::DynamicStruct& ds);

	//return true if user command can be fulfilled with low level commands
	bool expandUserCmdConnectDevice();
	bool expandUserCmdResetDevice();
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
		CommandState state;
	};
	ConsoleCommand _consoleCommand;

	ConsoleOperator * _pConsoleOperator;
};

#endif /* USERCOMMANDRUNNER_H_ */
