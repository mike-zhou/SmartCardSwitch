/*
 * CommandRunner.cpp
 *
 *  Created on: Oct 16, 2018
 *      Author: user1
 */

#include "CommandRunner.h"
#include "Poco/ScopedLock.h"
#include "Logger.h"

CommandRunner * CommandRunner::_pInstance = nullptr;
Poco::Mutex CommandRunner::_mutex;

extern Logger * pLogger;

CommandRunner::CommandRunner(): Task("CommandRunner")
{
	_state = CommandRunnerState::IDLE;
	_commandId = 0;
	_commandSucceeded = false;
	_pDeviceAccessor = nullptr;
}

CommandRunner * CommandRunner::GetInstance()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_pInstance == nullptr) {
		_pInstance = new CommandRunner;
	}

	return _pInstance;
}

void CommandRunner::SetDevice(DeviceAccessor * pDeviceAccessor)
{
	_pDeviceAccessor = pDeviceAccessor;
}

bool CommandRunner::AddObserver(ICommandRunnerObserver * pObserver)
{
	if(pObserver == nullptr) {
		pLogger->LogError("CommandRunner::AddObserver null parameter");
		return false;
	}
	else
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);

		auto observerIt = observerPtrArray.begin();
		for(; observerIt != observerPtrArray.end(); observerIt++) {
			if(*observerIt == pObserver) {
				break;
			}
		}
		if(observerIt != observerPtrArray.end()) {
			pLogger->LogError("CommandRunner::AddObserver observer has already existed");
			return false;
		}

		observerPtrArray.push_back(pObserver);
	}
	return true;
}

enum CommandRunner::CommandRunnerState CommandRunner::State()
{
	return _state;
}


bool CommandRunner::ExecuteCommand(const std::string& cmd, unsigned long cmdId, const std::string& expectedResult)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_state > CommandRunnerState::FINISHED) {
		return false;
	}
	else {
		_state = CommandRunnerState::COMMAND_READY;
		_command = cmd;
		_commandId = cmdId;
		_expectedResult = expectedResult;
		_actualResult.clear();

		_event.set();
	}

	return true;
}

void CommandRunner::CancelCommand()
{
	if(_state > CommandRunnerState::FINISHED) {
		_state = CommandRunnerState::CANCELLED;
	}
}

unsigned long CommandRunner::CommandId()
{
	return _commandId;
}

bool CommandRunner::CommandResult(unsigned long commandId, bool& bSuccessful, std::string& commandResult)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_state != CommandRunnerState::FINISHED) {
		return false;
	}

	if(commandId != _commandId) {
		return false;
	}

	bSuccessful = _commandSucceeded;
	commandResult = _actualResult;

	return true;
}

void CommandRunner::OnFeedback(const std::string& feedback)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	_feedbacks.push_back(feedback);
	_event.set();
}

void CommandRunner::processFeedBacks()
{
	if(_feedbacks.size() < 1) {
		return;
	}

}

void CommandRunner::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			if(_event.tryWait(10) == false) {
				continue;
			}
			else
			{
				Poco::ScopedLock<Poco::Mutex> lock(_mutex);
				if(_state == CommandRunnerState::COMMAND_READY)
				{
					//send out command
					if(_pDeviceAccessor->SendCommand(_command))
					{
						_state = CommandRunnerState::COMMAND_SENT;
					}
					else
					{
						pLogger->LogError("CommandRunner::runTask failed in sending: " + _command);
						_commandSucceeded = false;
						_state = CommandRunnerState::FINISHED;
					}
				}

				processFeedBacks();
			}
		}
	}
}

