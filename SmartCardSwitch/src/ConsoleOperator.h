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


class ConsoleOperator: public Poco::Task, public IResponseReceiver
{
public:
	ConsoleOperator(ICommandReception * pCmdReceiver);
	virtual ~ConsoleOperator() {}

private:
	//Poco::Task
	virtual void runTask() override;

	//IResponseReceiver
	virtual void OnDevicesGet(CommandKey key, bool bSuccess, const std::vector<std::string>& devices) override;
	virtual void OnStepperConfigStep(CommandKey key, bool bSuccess) override;
	virtual void OnStepperAccelerationBuffer(CommandKey key, bool bSuccess) override;
	virtual void OnStepperAccelerationBufferDecrement(CommandKey key, bool bSuccess) override;
	virtual void OnStepperDecelerationBuffer(CommandKey key, bool bSuccess)  override;
	virtual void OnStepperDecelerationBufferIncrement(CommandKey key, bool bSuccess) override;
	virtual void OnStepperConfigHome(CommandKey key, bool bSuccess) override;

private:
	const unsigned int BDC_AMOUNT = 6;
	const unsigned int STEPPER_AMOUNT = 5;
	const unsigned int LOCATOR_AMOUNT = 8;
	const unsigned int LOCATOR_LINE_NUMBER_INVALID = 0;
	const unsigned int LOCATOR_LINE_NUMBER_MIN = 1;
	const unsigned int LOCATOR_LINE_NUMBER_MAX = 8;

	enum Type
	{
		Invalid = -1,
		DevicesGet = 0,
		DeviceConnect = 1,
		DeviceQueryPower = 2,
		DeviceQueryFuse = 3,
		OptPowerOn = 20,
		OptPowerOff = 21,
		OptQueryPower = 22,
		DcmPowerOn = 30,
		DcmPowerOff = 31,
		DcmQueryPower =32,
		BdcsPowerOn = 40,
		BdcsPowerOff = 41,
		BdcsQueryPower = 42,
		BdcCoast = 43,
		BdcReverse = 44,
		BdcForward = 45,
		BdcBreak = 46,
		BdcQuery = 47,
		SteppersPowerOn = 60,
		SteppersPowerOff = 61,
		SteppersQueryPower = 62,
		StepperQueryResolution = 63,
		StepperConfigStep = 64,
		StepperAccelerationBuffer = 65,
		StepperAccelerationBufferDecrement = 66,
		StepperDecelerationBuffer = 67,
		StepperDecelerationBufferIncrement = 68,
		StepperEnable = 69,
		StepperForward = 70,
		StepperSteps = 71,
		StepperRun = 72,
		StepperConfigHome = 73,
		StepperMove = 74,
		StepperQuery = 75,
		StepperForceState = 76,
		LocatorQuery = 90,
		BdcDelay = 200,
		SaveMovementConfig = 300,
		LoadMovementConfigStepper = 301,
		SaveCoordinates = 350,
		LoadCoordinates = 351
	};

	std::deque<char> _input;

	ICommandReception * _pCommandReception;
	ICommandReception::CommandKey _cmdKey;
	bool _bCmdFinish;
	bool _bCmdSucceed;
	std::vector<std::string> _devices;
	struct StepperData
	{
		unsigned int homeOffset = 0;
		unsigned int steps = 0;
		bool enabled = true;
		bool forward = false;
	};
	std::vector<StepperData> _steppers;

	void processInput();
	void showHelp();
	void loadMovementConfig();
	void stepperMove(unsigned int index, bool forward, unsigned int steps);
};


#endif /* CONSOLEOPERATOR_H_ */
