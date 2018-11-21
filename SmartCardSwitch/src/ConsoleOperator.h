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

class ConsoleOperator: public Poco::Task, public IResponseReceiver
{
public:
	ConsoleOperator(ICommandReception * pCmdReceiver);
	virtual ~ConsoleOperator() {}

	void AddObserver(IResponseReceiver * pObserver);
	bool RunConsoleCommand(const std::string& command);

private:
	//Poco::Task
	virtual void runTask() override;

	//IResponseReceiver
	virtual void OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices) override;
	virtual void OnStepperConfigStep(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBuffer(CommandId key, bool bSuccess)  override;
	virtual void OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperRun(CommandId key, bool bSuccess) override;
	virtual void OnStepperConfigHome(CommandId key, bool bSuccess) override;
	virtual void OnStepperForward(CommandId key, bool bSuccess) override;
	virtual void OnStepperSteps(CommandId key, bool bSuccess) override;
	virtual void OnStepperQuery(CommandId key, bool bSuccess,
									StepperState state,
									bool bEnabled,
									bool bForward,
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
private:
	const unsigned int BDC_AMOUNT = 6;
	const unsigned int STEPPER_AMOUNT = 5;
	const unsigned int LOCATOR_AMOUNT = 8;
	const unsigned int LOCATOR_LINE_NUMBER_INVALID = 0;
	const unsigned int LOCATOR_LINE_NUMBER_MIN = 1;
	const unsigned int LOCATOR_LINE_NUMBER_MAX = 8;


	std::deque<char> _input;

	Poco::Mutex _mutex;

	std::vector<IResponseReceiver *> _observerPtrArray;

	ICommandReception * _pCommandReception;
	ICommandReception::CommandId _cmdKey;
	bool _bCmdFinish;
	bool _bCmdSucceed;
	int _queriedHomeOffset;
	std::vector<std::string> _devices;
	struct StepperData
	{
		long homeOffset = -1;
		unsigned int steps = 0;
		bool enabled = true;
		bool forward = false;
	};
	std::vector<StepperData> _steppers;

	std::string getConsoleCommand();
	void showHelp();
	void prepareRunning();
	void waitCommandFinish();

	void loadMovementConfig();
	void stepperSetState(unsigned int index, int state);
	void stepperMove(unsigned int index, bool forward, unsigned int steps);
	void saveCoordinates(int type, unsigned int index);
};


#endif /* CONSOLEOPERATOR_H_ */
