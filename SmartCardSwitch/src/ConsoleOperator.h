/*
 * ConsoleOperator.h
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#ifndef CONSOLEOPERATOR_H_
#define CONSOLEOPERATOR_H_

#include <string>
#include <deque>
#include <vector>

#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

#include "ICommandReception.h"
#include "ConsoleCommandFactory.h"

/**
 * This class is used to operator the device manually.
 * It reads command from console or other source,
 * calls corresponding functions in ICommandReception instance which is CommandRunner object,
 * process results from ICommandReception instance.
 */
class ConsoleOperator: public Poco::Task, public IResponseReceiver
{
public:
	ConsoleOperator(ICommandReception * pCmdReceiver);
	virtual ~ConsoleOperator() {}

	//API to accept external console commands.
	//On success, a valid command ID is returned so that the observer can filter out
	//the reply to specific command from replies.
	CommandId RunConsoleCommand(const std::string& command);
	void AddObserver(IResponseReceiver * pObserver);

private:
	//Poco::Task
	virtual void runTask() override;

	//IResponseReceiver
	virtual void OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices) override;
	virtual void OnDeviceConnect(CommandId key, bool bSuccess) override;
	virtual void OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn) override;
	virtual void OnDeviceDelay(CommandId key, bool bSuccess)  override;
	virtual void OnOptPowerOn(CommandId key, bool bSuccess)  override;
	virtual void OnOptPowerOff(CommandId key, bool bSuccess)  override;
	virtual void OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)  override;
	virtual void OnDcmPowerOn(CommandId key, bool bSuccess)  override;
	virtual void OnDcmPowerOff(CommandId key, bool bSuccess)  override;
	virtual void OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered)  override;
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
	virtual void OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs) override;
	virtual void OnStepperConfigStep(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBuffer(CommandId key, bool bSuccess)  override;
	virtual void OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperEnable(CommandId key, bool bSuccess) override;
	virtual void OnStepperForward(CommandId key, bool bSuccess) override;
	virtual void OnStepperSteps(CommandId key, bool bSuccess) override;
	virtual void OnStepperRun(CommandId key, bool bSuccess) override;
	virtual void OnStepperConfigHome(CommandId key, bool bSuccess) override;
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

	virtual void OnStepperForwardClockwise(CommandId key, bool bSuccess) override;
	virtual void OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput) override;

private:
	const unsigned int BDC_AMOUNT = 6;
	const unsigned int STEPPER_AMOUNT = 5;
	const unsigned int LOCATOR_AMOUNT = 8;
	const unsigned int LOCATOR_LINE_NUMBER_INVALID = 0;
	const unsigned int LOCATOR_LINE_NUMBER_MIN = 1;
	const unsigned int LOCATOR_LINE_NUMBER_MAX = 8;

	std::deque<char> _input;

	Poco::Mutex _upperMutex; //make sure that only ONE console command from console or UserCommandRunner is processed at any time.
	Poco::Mutex _lowerMutex; //synchronize interactions between this object and CommandRunner object.

	std::vector<IResponseReceiver *> _observerPtrArray;

	ICommandReception * _pCommandReception; //the instance where commands can be sent to.
	ICommandReception::CommandId _cmdKey;
	unsigned int _index;
	bool _bCmdFinish;
	bool _bCmdSucceed;
	std::vector<std::string> _devices;
	struct StepperData
	{
		StepperState state = StepperState::Unknown;

		long homeOffset = -1;
		unsigned int steps = 0;
		bool enabled = true;
		bool forward = false;
		bool forwardClockwise = true;

		int locatorIndex = 0;
		int locatorLineNumberStart = 0;
		int locatorLineNumberTerminal = 0;

		unsigned int lowClks = 1;
		unsigned int highClks = 1;
		unsigned int accelerationBuffer = 0;
		unsigned int accelerationBufferDecrement = 1;
		unsigned int decelerationBuffer = 0;
		unsigned int decelerationBufferIncrement = 1;
	};
	std::vector<StepperData> _steppers;

	struct BdcData
	{
		unsigned int lowClks = 0;
		unsigned int highClks = 0;
		unsigned int cycles = 0;
	};
	std::vector<BdcData> _bdcs;

	bool runConsoleCommand(const std::string& command, ICommandReception::CommandId & cmdId);

	std::string getConsoleCommand();
	void showHelp();
	void prepareRunning();
	void waitCommandFinish();

	enum MovementType
	{
		StepperBoundary = 0,
		StepperGeneral = 1,
		StepperCardInsert = 2,
		StepperGoHome = 3,
		Bdc = 20
	};
	void saveMovementConfig(MovementType type, unsigned int index);
	void saveCoordinates(int type, unsigned int index);

	void loadMovementConfig();

	//compound commands
	void stepperSetState(unsigned int index, int state);
	void stepperMove(const unsigned int index, const bool forward, const unsigned int steps);
};


#endif /* CONSOLEOPERATOR_H_ */
