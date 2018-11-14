/*
 * ConsoleOperator.h
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#ifndef CONSOLEOPERATOR_H_
#define CONSOLEOPERATOR_H_

#include <deque>

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
	virtual void runTask() override;

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
		LocatorQuery = 90,
		BdcDelay = 200,
		SaveMovementConfig = 300,
		LoadMovementConfigStepper = 301,
		SaveCoordinates = 350,
		LoadCoordinates = 351
	};

	ICommandReception * _pCommandReception;
	ICommandReception::CommandKey _cmdKey;
	std::deque<char> _input;

	void processInput();
	void showHelp();
};


#endif /* CONSOLEOPERATOR_H_ */
