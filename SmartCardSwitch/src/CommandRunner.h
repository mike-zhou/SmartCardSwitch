/*
 * CommandRunner.h
 *
 *  Created on: Oct 16, 2018
 *      Author: user1
 */

#ifndef COMMANDRUNNER_H_
#define COMMANDRUNNER_H_

#include <memory>
#include <string>
#include <deque>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "DeviceAccessor.h"
#include "Command.h"

struct CommandRunnerEvent
{
	enum Type
	{
		COMMAND_RESULT = 0,
		DEVICE_POWER_ON,
		DEVICE_POWER_OFF,
		DEVICE_CONNECTED,
		DEVICE_DISCONNECTED,
		PROXY_CONNECTED,
		PROXY_DISCONNECTED,
		STEPPER_OUT_OF_RANGE
	} type;

	union
	{
		struct
		{
			bool bSuccess;
			unsigned long commandId;
			std::string result;
		} commandResult;

		struct
		{
			unsigned int stepperIndex;
		} stepperOutOfRange;
	} data;
};


class ICommandRunnerObserver
{
	virtual void OnCommandRunnerEvent(const struct CommandRunnerEvent& event) = 0;

	virtual ~ICommandRunnerObserver() {}
};

class CommandRunner: public Poco::Task, public IDeviceObserver
{
public:
	enum CommandRunnerState
	{
		IDLE = 0,
		CANCELLED,
		FINISHED,
		COMMAND_READY,
		COMMAND_SENT
	};

	~CommandRunner() {}

	static CommandRunner * GetInstance();

	void SetDevice(DeviceAccessor * pDeviceAccessor);

	bool AddObserver(ICommandRunnerObserver * pObserver);

	enum CommandRunnerState State();

	bool ExecuteCommand(const std::string& cmd, unsigned long cmdId, const std::string& expectedResult);
	void CancelCommand();

	unsigned long CommandId();
	bool CommandResult(unsigned long commandId, bool& bSuccessful, std::string& commandResult);

private:
	//IDeviceObserver
	virtual void OnFeedback(const std::string& feedback) override;
	//Poco::Task
	void runTask();

protected:
	CommandRunner();

private:
	static CommandRunner * _pInstance;
	static Poco::Mutex _mutex;

	Poco::Event _event;
	enum CommandRunnerState _state;
	std::string _command;
	unsigned long _commandId;
	bool _commandSucceeded;
	std::string _expectedResult;
	std::string _actualResult;

	std::deque<std::string> _feedbacks;

	std::vector<ICommandRunnerObserver *> observerPtrArray;

	DeviceAccessor * _pDeviceAccessor;

	void processFeedBacks();
};



#endif /* COMMANDRUNNER_H_ */
