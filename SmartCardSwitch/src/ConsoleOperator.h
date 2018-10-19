/*
 * ConsoleOperator.h
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#ifndef CONSOLEOPERATOR_H_
#define CONSOLEOPERATOR_H_

#include <memory>
#include <string>
#include <deque>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "DeviceAccessor.h"
#include "Command.h"
#include "CommandFactory.h"

class ConsoleOperator: public Poco::Task, public IDeviceObserver
{
public:
	ConsoleOperator();

	void SetDevice(DeviceAccessor * pDeviceAccessor);

	void OnKeypressing(char key);

private:
	//IDeviceObserver
	virtual void OnFeedback(const std::string& feedback) override;
	//Poco::Task
	void runTask();

private:
	Poco::Mutex _mutex;

	Poco::Event _event;
	std::deque<char> _input;
	std::deque<std::string> _feedbacks;
	DeviceAccessor * _pDeviceAccessor;

	void processInput();
	void processFeedbacks();

	struct UserCommand
	{
		enum Type
		{
			Invalid = -1,
			DevicesGet = 0,
			DeviceConnect = 1,
			DeviceQueryPower = 2,
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
		} type;

		enum State
		{
			IDLE = 0,
			CANCELLED,
			FINISHED,
			COMMAND_READY,
			COMMAND_SENT
		} state;

		std::string command;
		unsigned long commandId;
		std::string expectedResult;

		//command data
		int bdcIndex;
		int stepperIndex;
		int locatorIndex;


	} _userCommand;

	class Keyboard: public Poco::Runnable
	{
	public:
		Keyboard(ConsoleOperator * pReceiver);
		virtual ~Keyboard() {}

		void Terminate();

	private:
		virtual void run() override;

		bool _terminate;
		ConsoleOperator * _pReceiver;
	};

	Keyboard * _pKeyboardObj;
	Poco::Thread _keyboardThread;
};



#endif /* CONSOLEOPERATOR_H_ */
