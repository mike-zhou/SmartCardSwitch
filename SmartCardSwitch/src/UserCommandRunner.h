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
 * expands one user command to multiple console commands,
 * passes those console commands to ConsoleOperator instance one by one,
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

	//IResponseReceiver
	virtual void OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices) override;
	virtual void OnDeviceConnect(CommandId key, bool bSuccess) override;
	virtual void OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)  override;
	virtual void OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn) override;
	virtual void OnDeviceDelay(CommandId key, bool bSuccess)  override;
 	virtual void OnOptPowerOn(CommandId key, bool bSuccess)  override;
	virtual void OnOptPowerOff(CommandId key, bool bSuccess)   override;
	virtual void OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnDcmPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnDcmPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered) override {}
	virtual void OnBdcsPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnBdcsPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnBdcCoast(CommandId key, bool bSuccess) override;
	virtual void OnBdcReverse(CommandId key, bool bSuccess) override;
	virtual void OnBdcForward(CommandId key, bool bSuccess) override;
	virtual void OnBdcBreak(CommandId key, bool bSuccess) override;
	virtual void OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status) override;
	virtual void OnSteppersPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnSteppersPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs) override {}
	virtual void OnStepperConfigStep(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperEnable(CommandId key, bool bSuccess) override;
	virtual void OnStepperForward(CommandId key, bool bSuccess) override;
	virtual void OnStepperSteps(CommandId key, bool bSuccess) override;
	virtual void OnStepperRun(CommandId key, bool bSuccess) override;
	virtual void OnStepperConfigHome(CommandId key, bool bSuccess) override;
	virtual void OnStepperMove(CommandId key, bool bSuccess) override {}
	virtual void OnStepperQuery(CommandId key, bool bSuccess,
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
								unsigned long decelerationBufferIncrement) override;

	virtual void OnStepperSetState(CommandId key, bool bSuccess) override {}
	virtual void OnStepperForwardClockwise(CommandId key, bool bSuccess) override;
	virtual void OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput) override;

private:
	static const int STEPPER_AMOUNT = 5;
	static const int LOCATOR_AMOUNT = 8;
	static const int DCM_AMOUNT = 2;

	const unsigned int STEPPER_X = 0;
	const unsigned int STEPPER_Y = 1;
	const unsigned int STEPPER_Z = 2;
	const unsigned int STEPPER_W = 3;

	////////////////////////////////////////
	// user command related data and functions
	////////////////////////////////////////

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

		//----user command parameters----
		//connect device
		std::string deviceName;
		//confirm reset
		unsigned int locatorIndexReset;
		unsigned int lineNumberReset;
		//smart card related command
		unsigned int smartCardNumber;
		//bar code related command
		unsigned int barCodeNumber;
		//keys
		unsigned int downPeriod;
		unsigned int upPeriod;
		std::vector<unsigned int> keyNumbers;
		//dcm
		unsigned int dcmIndex;

		//---- user command result ----
		bool smartCardReaderSlotOccupied;
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
	void finishUserCommandConnectDevice(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommandCheckResetPressed(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommandCheckResetReleased(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommandResetDevice(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommandInsertSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommandRemoveSmartCard(CommandState & updatedCmdState, std::string & updatedErrorInfo);
	void finishUserCommand(CommandState consoleCmdState, const std::string& errorInfo);

	//fill _userCommand with information in user command JSON
	void parseUserCmdConnectDevice(Poco::DynamicStruct& ds);
	void parseUserCmdCheckResetPressed(Poco::DynamicStruct& ds);
	void parseUserCmdCheckResetReleased(Poco::DynamicStruct& ds);
	void parseUserCmdResetDevice(Poco::DynamicStruct& ds);
	void parseUserCmdSmartCard(Poco::DynamicStruct& ds);
	void parseUserCmdSwipeSmartCard(Poco::DynamicStruct& ds);
	void parseUserCmdTapSmartCard(Poco::DynamicStruct& ds);
	void parseUserCmdBarCode(Poco::DynamicStruct& ds);
	void parseUserCmdKeys(Poco::DynamicStruct& ds);
	void parseUserCmdDcm(Poco::DynamicStruct& ds);

	//fulfill user command with console commands
	void executeUserCmdConnectDevice();
	void executeUserCmdCheckResetPressed();
	void executeUserCmdCheckResetReleased();
	void executeUserCmdResetDevice();
	void executeUserCmdInsertSmartCard();
	void executeUserCmdRemoveSmartCard();
	void executeUserCmdSwipeSmartCard();
	void executeUserCmdTapSmartCard();
	void executeUserCmdShowBarCode();
	void executeUserCmdPressPedKey();
	void executeUserCmdPressSoftKey();
	void executeUserCmdPressAssistKey();
	void executeUserCmdTouchScreen();

	enum class CurrentPosition
	{
		Unknown = 0,
		Home,
		SmartCardGate,
		BarCodeCardGate,
		SoftKeyGate,
		PedKeyGate,
		AssistKeyGate,
		TouchScreenGate,
		SmartCardReaderGate,
		ContactlessReaderGate,
		BarCodeReaderGate
	};
	CurrentPosition getCurrentPosition();

	int currentX();
	int currentY();
	int currentZ();
	int currentW();
	int currentV();

	//append commands to configure stepper movement
	void configStepperMovement(unsigned int index,
							unsigned int lowClks,
							unsigned int highClks,
							unsigned int accelerationBuffer,
							unsigned int accelerationBufferDecrement,
							unsigned int decelerationBuffer,
							unsigned int decelerationBufferIncrement);

	//append commands to move stepper index from initialPos to finalPos.
	void moveStepper(unsigned int index, unsigned int initialPos, unsigned int finalPos);
	void moveStepperX(unsigned int initialPos, unsigned int finalPos) { moveStepper(0, initialPos, finalPos); }
	void moveStepperY(unsigned int initialPos, unsigned int finalPos) { moveStepper(1, initialPos, finalPos); }
	void moveStepperZ(unsigned int initialPos, unsigned int finalPos) { moveStepper(2, initialPos, finalPos); }
	void moveStepperW(unsigned int initialPos, unsigned int finalPos) { moveStepper(3, initialPos, finalPos); }
	void moveStepperV(unsigned int initialPos, unsigned int finalPos) { moveStepper(4, initialPos, finalPos); }

	//smart card bay
	void moveSmartCardCarriage(unsigned int cardNumber);
	void pushUpSmartCardArm();
	void pullDownSmartCardArm();
	void releaseSmartCardArm();

	//delay
	void deviceDelay(unsigned int clks);
	//clamp operation
	void openClamp();
	void closeClamp();
	void releaseClamp();
	//movement between smart card and gate
	void gate_smartCard_withoutCard(unsigned int cardNumber);
	void gate_smartCard_withCard(unsigned int cardNumber);
	void smartCard_gate_withCard(unsigned int cardNumber);
	void smartCard_gate_withoutCard(unsigned int cardNumber);
	//keyPressingArm operation
	void pullUpKeyPressingArm();
	void putDownKeyPressingArm();
	void releaseKeyPressingArm();
	//movement between PED keys and gate
	void pedKey_gate(unsigned int keyNumber);
	void pedKey_pedKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	void gate_pedKey(unsigned int keyNumber);
	//movement between softkey and gate
	void softKey_gate(unsigned int keyNumber);
	void softKey_softKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	void gate_softKey(unsigned int keyNumber);
	//movement between assist key and gate
	void assistKey_gate(unsigned int keyNumber);
	void assistKey_assistKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	void gate_assistKey(unsigned int keyNumber);
	//movement between touch screen area and gate
	void touchScreenKey_gate(unsigned int keyNumber);
	void touchScreenKey_touchScreenKey(unsigned int keyNumberFrom, unsigned int keyNumberTo);
	void gate_touchScreenKey(unsigned int keyNumber);
	//movement between smart card reader and gate
	void smartCardReader_gate_withCard();
	void smartCardReader_gate_withoutCard();
	void gate_smartCardReader_withCard();
	void gate_smartCardReader_withoutCard();
	//movement between contactless reader and gate
	void contactlessReader_gate();
	void gate_contactlessReader();
	//movement between barcode reader and gate
	void barcodeReader_gate();
	void gate_barcodeReader();
	//from Gate to Gate
	void gateToGate(unsigned int fromX, unsigned int fromY, unsigned int fromZ, unsigned int fromW,
					unsigned int toX, unsigned int toY, unsigned int toZ, unsigned int toW);
	void toHome();
	void toSmartCardGate();
	void toPedKeyGate();
	void toSoftKeyGate();
	void toAssistKeyGate();
	void toTouchScreenGate();
	void toSmartCardReaderGate();
	void toContactlessReaderGate();
	void toBarcodeReaderGate();

	void powerOnOpt(bool on);
	void powerOnDcm(bool on, unsigned int index);

	std::vector<IUserCommandRunnerObserver *> _observerPtrArray;

	//////////////////////////////////////
	// console command data and functions
	//////////////////////////////////////

	Poco::Mutex _consoleCommandMutex;
	struct ConsoleCommand
	{
		ICommandReception::CommandId cmdId;
		CommandState state;

		//params
		unsigned int stepperIndex;
		unsigned int steps;
		bool stepperForward;
		bool stepperForwardClockwise;
		unsigned int locatorIndex;

		//command results
		std::vector<std::string> resultDevices;
		bool resultDeviceConnected;
		bool resultDevicePowered;
		bool resultDeviceFuseOk;
		bool resultOptPowered;
		bool resultBdcsPowered;
		bool resultSteppersPowered;
		struct StepperStatus
		{
			StepperState state;
			bool enabled;
			bool forward;
			bool forwardClockwise;
			unsigned int homeOffset;
			unsigned int targetPosition;
			unsigned int locatorIndex;
			unsigned int locatorLineNumberStart;
			unsigned int locatorLineNumberTerminal;
		} resultSteppers[STEPPER_AMOUNT];
		unsigned char resultLocators[LOCATOR_AMOUNT];
	};
	ConsoleCommand _consoleCommand;

	void throwError(const std::string& errorInfo);
	void setConsoleCommandParameter(const std::string & cmd);
	void runConsoleCommand(const std::string& cmd);

	ConsoleOperator * _pConsoleOperator;
};

#endif /* USERCOMMANDRUNNER_H_ */
