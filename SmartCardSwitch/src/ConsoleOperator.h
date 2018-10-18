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
			OptPowerOn = 3,
			OptPowerOff = 4,
			OptQueryPower = 5,
			DcmPowerOn = 6,
			DcmPowerOff = 7,
			DcmQueryPower,
			BdcsPowerOn,
			BdcsPowerOff,
			BdcsQueryPower,
			BdcCoast,
			BdcReverse,
			BdcForward,
			BdcBreak,
			BdcQuery,
			SteppersPowerOn,
			SteppersPowerOff,
			SteppersQueryPower,
			StepperQueryResolution,
			StepperConfigStep,
			StepperAccelerationBuffer,
			StepperAccelerationBufferDecrement,
			StepperDecelerationBuffer,
			StepperDecelerationBufferIncrement,
			StepperEnable,
			StepperForward,
			StepperSteps,
			StepperRun,
			StepperConfigHome,
			StepperMove,
			StepperQuery,
			LocatorQuery,
		} type;

		enum State
		{
			IDLE = 0,
			CANCELLED,
			FINISHED,
			COMMAND_READY,
			COMMAND_SENT
		} state;

		unsigned long commandId;

		//command data
		int bdcIndex;
		int stepperIndex;

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
