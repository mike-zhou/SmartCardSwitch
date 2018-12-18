/*
 * ConsoleOperator.cpp
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#include "stdio.h"
#include <iostream>

#include "ConsoleOperator.h"
#include "Logger.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

extern Logger * pLogger;
extern CoordinateStorage * pCoordinateStorage;
extern MovementConfiguration * pMovementConfiguration;

ConsoleOperator::ConsoleOperator(ICommandReception * pCmdReceiver) : Task("ConsoleOperator")
{
	_pCommandReception = pCmdReceiver;
	_cmdKey = InvalidCommandId;
	_bCmdFinish = true;
	_bCmdSucceed = false;
	_queriedHomeOffset = -1;
	_index = 0;

	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		StepperData s;
		_steppers.push_back(s);
	}

	for(unsigned int i=0; i<BDC_AMOUNT; i++) {
		BdcData b;
		_bdcs.push_back(b);
	}
}

void ConsoleOperator::showHelp()
{
	std::cout << "\r\n=========== HELP: Command format=========\r\n";
	std::cout << ConsoleCommandFactory::GetHelp();
	std::cout << "===============================================\r\n";
}

void ConsoleOperator::prepareRunning()
{
	_bCmdSucceed = false;
	_bCmdFinish = false;
}

void ConsoleOperator::waitCommandFinish()
{
	for(;;) {
		if(_bCmdFinish) {
			break;
		}
		sleep(1);
	}
}

void ConsoleOperator::loadMovementConfig()
{
	long lowClks ;
	long highClks ;
	long accelerationBuffer ;
	long accelerationBufferDecrement ;
	long decelerationBuffer ;
	long decelerationBufferIncrement ;
	int locatorIndex ;
	int locatorLineNumberStart ;
	int locatorLineNumberTerminal;

	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		auto rc = pMovementConfiguration->GetStepperGeneral(i,
															lowClks,
															highClks,
															accelerationBuffer,
															accelerationBufferDecrement,
															decelerationBuffer,
															decelerationBufferIncrement);

		rc = rc && pMovementConfiguration->GetStepperBoundary(i,locatorIndex,
				locatorLineNumberStart,
				locatorLineNumberTerminal);

		if(rc == false)
		{
			pLogger->LogError("ConsoleOperator::loadMovementConfig cannot retrieve settings of stepper: " + std::to_string(i));
		}
		else
		{
			//load step
			prepareRunning();
			_cmdKey = _pCommandReception->StepperConfigStep(i, lowClks, highClks);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config step for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config step for stepper: " + std::to_string(i));
			}

			//load acceleration buffer
			prepareRunning();
			_cmdKey = _pCommandReception->StepperAccelerationBuffer(i, accelerationBuffer);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config accelerationBuffer for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config accelerationBuffer for stepper: " + std::to_string(i));
			}

			//load acceleration buffer decrement
			prepareRunning();
			_cmdKey = _pCommandReception->StepperAccelerationBufferDecrement(i, accelerationBufferDecrement);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config accelerationBufferDecrement for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config accelerationBufferDecrement for stepper: " + std::to_string(i));
			}

			//load deceleration buffer
			prepareRunning();
			_cmdKey = _pCommandReception->StepperDecelerationBuffer(i, decelerationBuffer);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config decelerationBuffer for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config decelerationBuffer for stepper: " + std::to_string(i));
			}

			//load deceleration buffer increment
			prepareRunning();
			_cmdKey = _pCommandReception->StepperDecelerationBufferIncrement(i, decelerationBufferIncrement);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config decelerationBufferIncrement for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config decelerationBufferIncrement for stepper: " + std::to_string(i));
			}

			//load locator
			prepareRunning();
			_cmdKey = _pCommandReception->StepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			waitCommandFinish();
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config locator for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config locator for stepper: " + std::to_string(i));
			}
		}
	}
}

void ConsoleOperator::stepperSetState(unsigned int index, int state)
{
	ICommandReception::StepperState stepperState;

	switch(state)
	{
	case 4:
		stepperState = ICommandReception::StepperState::KnownPosition;
		break;

	default:
		pLogger->LogError("ConsoleOperator::stepperSetState unsupported stepper state: " + std::to_string(state));
		return;
	}

	//prepareRunning();
	_pCommandReception->StepperSetState(index, stepperState);
	//waitCommandFinish();

//	if(_bCmdSucceed) {
//		pLogger->LogInfo("ConsoleOperator::stepperSetState succeeded in setting stepper state to " + std::to_string(state));
//		_steppers[index].homeOffset = 0;
//	}
//	else {
//		pLogger->LogInfo("ConsoleOperator::stepperSetState failed in setting stepper state to " + std::to_string(state));
//	}
}

void ConsoleOperator::stepperMove(unsigned int index, bool forward, unsigned int steps)
{
	long finalPos;
	StepperData data;

	if(index > STEPPER_AMOUNT) {
		pLogger->LogError("ConsoleOperator::stepperMove wrong stepper index: " + std::to_string(index));
		return;
	}

	data = _steppers[index];
	if(forward) {
		finalPos = data.homeOffset + steps;
	}
	else {
		finalPos = data.homeOffset - steps;
	}
	if(finalPos < 0) {
		pLogger->LogError("ConsoleOperator::stepperMove final position out of home, current position: " + std::to_string(data.homeOffset));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::stepperMove from " + std::to_string(data.homeOffset) + " to " + std::to_string(finalPos));

	// set stepper direction
	prepareRunning();
	_pCommandReception->StepperForward(index, forward);
	waitCommandFinish();
	if(!_bCmdSucceed) {
		pLogger->LogError("ConsoleOperator::stepperMove failed to set stepper direction: " + std::to_string(index));
		return;
	}
	_steppers[index].forward = forward;

	// set steps
	prepareRunning();
	_pCommandReception->StepperSteps(index, steps);
	waitCommandFinish();
	if(!_bCmdSucceed) {
		pLogger->LogError("ConsoleOperator::stepperMove failed to set steps: " + std::to_string(index));
		return;
	}

	//move stepper
	prepareRunning();
	_pCommandReception->StepperRun(index, data.homeOffset, finalPos);
	waitCommandFinish();
	if(!_bCmdSucceed) {
		pLogger->LogError("ConsoleOperator::stepperMove failed to move stepper : " + std::to_string(index));
	}

	//query stepper
	prepareRunning();
	_pCommandReception->StepperQuery(index);
	waitCommandFinish();
	if(!_bCmdSucceed) {
		pLogger->LogError("ConsoleOperator::stepperMove failed to query stepper : " + std::to_string(index));
		return;
	}
	_steppers[index].homeOffset = _queriedHomeOffset; //update home offset

	if(finalPos != _steppers[index].homeOffset)
	{
		pLogger->LogError("ConsoleOperator::stepperMove stepper actually moved to: " + std::to_string(_steppers[index].homeOffset) + " expected: " + std::to_string(finalPos));
	}

	pLogger->LogInfo("ConsoleOperator::stepperMove moved from " + std::to_string(data.homeOffset) + " to " + std::to_string(_steppers[index].homeOffset));
}

void ConsoleOperator::saveMovementConfig(MovementType type, unsigned int index)
{
	switch(type)
	{
		case MovementType::StepperBoundary:
		{
			if(index >= STEPPER_AMOUNT) {
				pLogger->LogError("ConsoleOperator::saveMovementConfig stepper index out of range: " + std::to_string(index));
			}
			else
			{
				unsigned int locatorIndex = _steppers[index].locatorIndex;
				unsigned int locatorLineNumberStart = _steppers[index].locatorLineNumberStart;
				unsigned int locatorLineNumberTerminal = _steppers[index].locatorLineNumberTerminal;

				if(!pMovementConfiguration->SetStepperBoundary(index, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal)) {
					pLogger->LogError("ConsoleOperator::saveMovementConfig failed in stepper boundary saving: " + std::to_string(index));
				}
			}
		}
		break;
		case MovementType::StepperGeneral:
		{
			if(index >= STEPPER_AMOUNT) {
				pLogger->LogError("ConsoleOperator::saveMovementConfig stepper index out of range: " + std::to_string(index));
			}
			else
			{
				unsigned int lowClks = _steppers[index].lowClks;
				unsigned int highClks = _steppers[index].highClks;
				unsigned int accelerationBuffer = _steppers[index].accelerationBuffer;
				unsigned int accelerationBufferDecrement = _steppers[index].accelerationBufferDecrement;
				unsigned int decelerationBuffer = _steppers[index].decelerationBuffer;
				unsigned int decelerationBufferIncrement = _steppers[index].decelerationBufferIncrement;

				if(!pMovementConfiguration->SetStepperGeneral(index, lowClks, highClks, accelerationBuffer,
						accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement)) {
					pLogger->LogError("ConsoleOperator::saveMovementConfig failed in stepper general movement saving: " + std::to_string(index));
				}
			}
		}
		break;
		case MovementType::StepperCardInsert:
		{
			if(index >= STEPPER_AMOUNT) {
				pLogger->LogError("ConsoleOperator::saveMovementConfig stepper index out of range: " + std::to_string(index));
			}
			else
			{
				//save card insert config of designated stepper as default.
				unsigned int lowClks = _steppers[index].lowClks;
				unsigned int highClks = _steppers[index].highClks;
				unsigned int accelerationBuffer = _steppers[index].accelerationBuffer;
				unsigned int accelerationBufferDecrement = _steppers[index].accelerationBufferDecrement;
				unsigned int decelerationBuffer = _steppers[index].decelerationBuffer;
				unsigned int decelerationBufferIncrement = _steppers[index].decelerationBufferIncrement;

				if(!pMovementConfiguration->SetStepperCardInsert(lowClks, highClks, accelerationBuffer,
						accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement)) {
					pLogger->LogError("ConsoleOperator::saveMovementConfig failed in stepper insert movement saving");
				}
			}
		}
		break;
		case MovementType::StepperGoHome:
		{
			if(index >= STEPPER_AMOUNT) {
				pLogger->LogError("ConsoleOperator::saveMovementConfig stepper index out of range: " + std::to_string(index));
			}
			else
			{
				//save home config of designated stepper as default.
				unsigned int lowClks = _steppers[index].lowClks;
				unsigned int highClks = _steppers[index].highClks;
				unsigned int accelerationBuffer = _steppers[index].accelerationBuffer;
				unsigned int accelerationBufferDecrement = _steppers[index].accelerationBufferDecrement;
				unsigned int decelerationBuffer = _steppers[index].decelerationBuffer;
				unsigned int decelerationBufferIncrement = _steppers[index].decelerationBufferIncrement;

				if(!pMovementConfiguration->SetStepperGoHome(lowClks, highClks, accelerationBuffer,
						accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement)) {
					pLogger->LogError("ConsoleOperator::saveMovementConfig failed in stepper home movement saving");
				}
			}
		}
		break;
		case MovementType::Bdc:
		{
			if(index >= BDC_AMOUNT) {
				pLogger->LogError("ConsoleOperator::saveMovementConfig bdc index out of range: " + std::to_string(index));
			}
			else
			{
				//save config of designated bdc as default.
				unsigned int lowClks = _bdcs[index].lowClks;
				unsigned int highClks = _bdcs[index].highClks;
				unsigned int cycles = _bdcs[index].cycles;

				pMovementConfiguration->SetBdcConfig(lowClks, highClks, cycles);
			}
		}
		break;
		default:
		{
			pLogger->LogError("ConsoleOperator::saveMovementConfig unknown movement type: " + std::to_string((int)type));
		}
		break;
	}

	if(!pMovementConfiguration->PersistToFile()) {
		pLogger->LogError("ConsoleOperator::saveMovementConfig failed to persist movement");
	}
}

void ConsoleOperator::saveCoordinates(int type, unsigned int index)
{
	CoordinateStorage::Type coType = (CoordinateStorage::Type)type;

	auto success = pCoordinateStorage->SetCoordinate(coType,
											_steppers[0].homeOffset,
											_steppers[1].homeOffset,
											_steppers[2].homeOffset,
											_steppers[3].homeOffset,
											index);

	if(success) {
		pLogger->LogInfo("ConsoleOperator::saveCoordinates succeeded in saving coordinate");
	}
	else{
		pLogger->LogInfo("ConsoleOperator::saveCoordinates failed in saving coordinate");
	}

	if(success)
	{
		auto rc = pCoordinateStorage->PersistToFile();
		if(rc) {
			pLogger->LogError("ConsoleOperator::saveCoordinates succeeded in persisting file");
		}
		else {
			pLogger->LogError("ConsoleOperator::saveCoordinates failed in persisting file");
		}
	}
}

std::string ConsoleOperator::getConsoleCommand()
{
	std::string command;

	if(_input.size() < 1) {
		return command;
	}

	bool bCommandAvailable = false;

	//find '\n' in the input
	for(auto it=_input.begin(); it!=_input.end(); it++)
	{
		if(*it == '\n') {
			bCommandAvailable = true;
			break;
		}
	}
	if(!bCommandAvailable) {
		return command;
	}
	//retrieve command from beginning of _input
	for(;;)
	{
		auto c = _input.front();
		_input.pop_front();
		if(c == '\n') {
			break;
		}
		command.push_back(c);
	}

	//validate command characters
	bool bCmdValid = true;
	for(auto it=command.begin(); it!=command.end(); it++)
	{
		if((*it != ' ') && ((*it < '0') || (*it > '9'))) {
			bCmdValid = false;
			break;
		}
	}
	if(bCmdValid == false) {
		pLogger->LogError("ConsoleOperator::processInput invalid command: " + command);
		showHelp();
		command.clear();
	}

	return command;
}

bool ConsoleOperator::runConsoleCommand(const std::string& command)
{
	//parse command
	int d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;

	d0 = -1;
	d1 = -1;
	d2 = -1;
	d3 = -1;
	d4 = -1;
	d5 = -1;
	d6 = -1;
	d7 = -1;
	d8 = -1;
	d9 = -1;

	Poco::ScopedLock<Poco::Mutex> lock(_upperMutex);

	try
	{
		pLogger->LogInfo("ConsoleOperator::RunConsoleCommand command: " + command);
		sscanf(command.data(), "%d %d %d %d %d %d %d %d %d %d\n", &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
	}
	catch(...)
	{
		std::string e = "ConsoleOperator::RunConsoleCommand exception in parsing input";
		pLogger->LogError(e);
		return false;
	}

	//run command
	bool bKnownCmd = true;
	switch((ConsoleCommandFactory::Type)d0)
	{
		case ConsoleCommandFactory::Type::DevicesGet:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->DevicesGet();
		}
		break;

		case ConsoleCommandFactory::Type::DeviceConnect:
		{
			if(_devices.empty()) {
				pLogger->LogError("ConsoleOperator::RunConsoleCommand no device to connect");
			}
			else
			{
				Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
				int deviceNumber = d1;

				_cmdKey = _pCommandReception->DeviceConnect(deviceNumber);
			}
		}
		break;

		case ConsoleCommandFactory::Type::DeviceQueryPower:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->DeviceQueryPower();
		}
		break;

		case ConsoleCommandFactory::Type::DeviceQueryFuse:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->DeviceQueryFuse();
		}
		break;

		case ConsoleCommandFactory::Type::DeviceDelay:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int clks = d1;

			_cmdKey = _pCommandReception->DeviceDelay(clks);
		}
		break;

		case ConsoleCommandFactory::Type::BdcsPowerOn:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->BdcsPowerOn();
		}
		break;

		case ConsoleCommandFactory::Type::BdcsPowerOff:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->BdcsPowerOff();
		}
		break;

		case ConsoleCommandFactory::Type::BdcsQueryPower:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->BdcsQueryPower();
		}
		break;

		case ConsoleCommandFactory::Type::BdcCoast:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;

			_cmdKey = _pCommandReception->BdcCoast(index);
		}
		break;

		case ConsoleCommandFactory::Type::BdcReverse:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int lowClks = d2;
			unsigned int highClks = d3;
			unsigned int cycles = d4;

			_cmdKey = _pCommandReception->BdcReverse(index, lowClks, highClks, cycles);
			if(_cmdKey != InvalidCommandId)
			{
				_bdcs[index].lowClks = lowClks;
				_bdcs[index].highClks = highClks;
				_bdcs[index].cycles = cycles;
			}
		}
		break;

		case ConsoleCommandFactory::Type::BdcForward:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int lowClks = d2;
			unsigned int highClks = d3;
			unsigned int cycles = d4;

			_cmdKey = _pCommandReception->BdcForward(index, lowClks, highClks, cycles);
			if(_cmdKey != InvalidCommandId)
			{
				_bdcs[index].lowClks = lowClks;
				_bdcs[index].highClks = highClks;
				_bdcs[index].cycles = cycles;
			}
		}
		break;

		case ConsoleCommandFactory::Type::BdcBreak:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;

			_cmdKey = _pCommandReception->BdcBreak(index);
		}
		break;

		case ConsoleCommandFactory::Type::BdcQuery:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;

			_cmdKey = _pCommandReception->BdcQuery(index);
		}
		break;

		case ConsoleCommandFactory::Type::SteppersPowerOn:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->SteppersPowerOn();
		}
		break;

		case ConsoleCommandFactory::Type::SteppersPowerOff:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->SteppersPowerOff();
		}
		break;

		case ConsoleCommandFactory::Type::SteppersQueryPower:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->SteppersQueryPower();
		}
		break;

		case ConsoleCommandFactory::Type::StepperQueryResolution:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_cmdKey = _pCommandReception->StepperQueryResolution();
		}
		break;

		case ConsoleCommandFactory::Type::StepperConfigStep:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int lowClks = d2;
			unsigned int highClks = d3;

			_cmdKey = _pCommandReception->StepperConfigStep(index, lowClks, highClks);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].lowClks = lowClks;
				_steppers[index].highClks = highClks;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperAccelerationBuffer:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperAccelerationBuffer(index, value);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].accelerationBuffer = value;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperAccelerationBufferDecrement:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperAccelerationBufferDecrement(index, value);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].accelerationBufferDecrement = value;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperDecelerationBuffer:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperDecelerationBuffer(index, value);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].decelerationBuffer = value;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperDecelerationBufferIncrement:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperDecelerationBufferIncrement(index, value);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].decelerationBufferIncrement = value;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperEnable:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			bool enable = (d2 != 0);

			_cmdKey = _pCommandReception->StepperEnable(index, enable);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].enabled = enable;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperForward:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			bool forward = (d2 != 0);

			_cmdKey = _pCommandReception->StepperForward(index, forward);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].forward = forward;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperSteps:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperSteps(index, value);
		}
		break;

		case ConsoleCommandFactory::Type::StepperRun:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int initialPos = d2;
			unsigned int finalPos = d3;

			_cmdKey = _pCommandReception->StepperRun(index, initialPos, finalPos);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].homeOffset = finalPos;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperConfigHome:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			unsigned int locatorIndex = d2;
			unsigned int lineNumberStart = d3;
			unsigned int lineNumberTerminal = d4;

			_cmdKey = _pCommandReception->StepperConfigHome(index, locatorIndex, lineNumberStart, lineNumberTerminal);
			if(_cmdKey != InvalidCommandId)
			{
				_steppers[index].locatorIndex = locatorIndex;
				_steppers[index].locatorLineNumberStart = lineNumberStart;
				_steppers[index].locatorLineNumberTerminal = lineNumberTerminal;
			}
		}
		break;

		case ConsoleCommandFactory::Type::StepperMove:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			bool forward = (d2 != 0);
			unsigned int steps = d3;

			stepperMove(index, forward, steps);
			_cmdKey = InvalidCommandId;
		}
		break;

		case ConsoleCommandFactory::Type::StepperSetState:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;
			int state = d2;

			stepperSetState(index, state);
			_cmdKey = InvalidCommandId;
		}
		break;

		case ConsoleCommandFactory::Type::StepperQuery:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			_index = d1;

			_cmdKey = _pCommandReception->StepperQuery(_index);
		}
		break;

		case ConsoleCommandFactory::Type::LocatorQuery:
		{
			Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);
			unsigned int index = d1;

			_cmdKey = _pCommandReception->LocatorQuery(index);
		}
		break;

		case ConsoleCommandFactory::Type::BdcConfig:
		{
			unsigned int lowClks = _bdcs[0].lowClks;
			unsigned int highClks = _bdcs[0].highClks;
			unsigned int cycles = _bdcs[0].cycles;

			pMovementConfiguration->SetBdcConfig(lowClks, highClks, cycles);

			_cmdKey = InvalidCommandId;
		}
		break;

		case ConsoleCommandFactory::Type::SaveMovementConfig:
		{
			MovementType type = (MovementType)d1;
			int index = d2;

			//_cmdKey = _pCommandReception->SaveMovementConfig();
			saveMovementConfig(type, index);
			_cmdKey = InvalidCommandId;
		}
		break;

		case ConsoleCommandFactory::Type::LoadMovementConfigStepper:
		{
			loadMovementConfig();
			_cmdKey = InvalidCommandId;
		}
		break;

		case ConsoleCommandFactory::Type::SaveCoordinates:
		{
			int type = d1;
			unsigned int index = d2;

			saveCoordinates(type, index);
			_cmdKey = InvalidCommandId;
		}
		break;

		default:
		{
			bKnownCmd = false;
			pLogger->LogError("ConsoleOperator::RunConsoleCommand unknown command: " + command);
			showHelp();
		}
		break;
	}

	if((bKnownCmd) && (_cmdKey == InvalidCommandId)) {
		pLogger->LogInfo("ConsoleOperator::RunConsoleCommand no reply will be returned");
		return true;
	}
	else {
		return false;
	}
}

ICommandReception::CommandId ConsoleOperator::RunConsoleCommand(const std::string& command)
{
	bool success = runConsoleCommand(command);

	if(success) {
		return _cmdKey;
	}
	else {
		return ICommandReception::ICommandDataTypes::InvalidCommandId;
	}
}

void ConsoleOperator::OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDevicesGet unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = false;

	if(bSuccess != true) {
		pLogger->LogError("ConsoleOperator::OnDevicesGet failed");
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnDevicesGet devices amount: " + std::to_string(devices.size()));
	for(unsigned int i = 0; i < devices.size(); i++) {
		pLogger->LogInfo("ConsoleOperator::OnDevicesGet " + std::to_string(i) + ": " + devices[i]);
	}

	_devices = devices;
	_cmdKey = InvalidCommandId;
	_bCmdSucceed = true;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDevicesGet(key, bSuccess, devices);
	}
}

void ConsoleOperator::OnDeviceConnect(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDeviceConnect unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDeviceConnect(key, bSuccess);
	}
}

void ConsoleOperator::OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDeviceQueryPower unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDeviceQueryPower(key, bSuccess, bPowered);
	}
}

void ConsoleOperator::OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDeviceQueryFuse unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDeviceQueryFuse(key, bSuccess, bFuseOn);
	}
}

void ConsoleOperator::OnDeviceDelay(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDeviceDelay unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDeviceDelay(key, bSuccess);
	}
}

void ConsoleOperator::OnOptPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnOptPowerOn unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnOptPowerOn(key, bSuccess);
	}
}

void ConsoleOperator::OnOptPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnOptPowerOff unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnOptPowerOff(key, bSuccess);
	}
}

void ConsoleOperator::OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnOptQueryPower unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnOptQueryPower(key, bSuccess, bPowered);
	}
}

void ConsoleOperator::OnDcmPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDcmPowerOn unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDcmPowerOn(key, bSuccess);
	}
}

void ConsoleOperator::OnDcmPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDcmPowerOff unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDcmPowerOff(key, bSuccess);
	}
}

void ConsoleOperator::OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnDcmQueryPower unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnDcmQueryPower(key, bSuccess, bPowered);
	}
}

void ConsoleOperator::OnBdcsPowerOn(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcsPowerOn unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcsPowerOn(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcsPowerOff(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcsPowerOff unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcsPowerOff(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcsQueryPower unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcsQueryPower(key, bSuccess, bPowered);
	}
}

void ConsoleOperator::OnBdcCoast(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcCoast unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcCoast(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcReverse(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcReverse unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcReverse(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcForward(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcForward unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcForward(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcBreak(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcBreak unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcBreak(key, bSuccess);
	}
}

void ConsoleOperator::OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status)
{
	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnBdcQuery unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnBdcQuery(key, bSuccess, status);
	}

}

void ConsoleOperator::OnSteppersPowerOn(CommandId key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnSteppersPowerOn unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnSteppersPowerOn(key, bSuccess);
	}
}

void ConsoleOperator::OnSteppersPowerOff(CommandId key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnSteppersPowerOff unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnSteppersPowerOff(key, bSuccess);
	}
}

void ConsoleOperator::OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnSteppersQueryPower unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnSteppersQueryPower(key, bSuccess, bPowered);
	}
}

void ConsoleOperator::OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperQueryResolution unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	_bCmdFinish = true;
	_bCmdSucceed = bSuccess;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperQueryResolution(key, bSuccess, resolutionUs);
	}
}

void ConsoleOperator::OnStepperConfigStep(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperConfigStep unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperConfigStep finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperConfigStep(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperAccelerationBuffer(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperAccelerationBuffer unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperAccelerationBuffer finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperAccelerationBuffer(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperAccelerationBufferDecrement unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperAccelerationBufferDecrement finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperAccelerationBufferDecrement(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperDecelerationBuffer(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperDecelerationBuffer unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperDecelerationBuffer finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperDecelerationBuffer(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperDecelerationBufferIncrement unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperDecelerationBufferIncrement finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperDecelerationBufferIncrement(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperRun(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperRun unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperRun finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperRun(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperConfigHome(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperConfigHome unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperConfigHome finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperConfigHome(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperEnable(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperEnable unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperEnable finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperEnable(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperForward(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperForward unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperForward finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperForward(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperSteps(CommandId key, bool bSuccess)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperSteps unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperSteps finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperSteps(key, bSuccess);
	}
}

void ConsoleOperator::OnStepperQuery(CommandId key, bool bSuccess,
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
									unsigned long decelerationBufferIncrement)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperQuery unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperQuery finished");
	_bCmdSucceed = bSuccess;
	_queriedHomeOffset = homeOffset;
	_bCmdFinish = true;

	if(_bCmdSucceed)
	{
		_steppers[_index].state = state;
		_steppers[_index].enabled = bEnabled;
		_steppers[_index].forward = bForward;

		_steppers[_index].locatorIndex = locatorIndex;
		_steppers[_index].locatorLineNumberStart = locatorLineNumberStart;
		_steppers[_index].locatorLineNumberTerminal = locatorLineNumberTerminal;
		_steppers[_index].homeOffset = homeOffset;
		_steppers[_index].lowClks = lowClks;
		_steppers[_index].highClks = highClks;
		_steppers[_index].accelerationBuffer = accelerationBuffer;
		_steppers[_index].accelerationBufferDecrement = accelerationBufferDecrement;
		_steppers[_index].decelerationBuffer = decelerationBuffer;
		_steppers[_index].decelerationBufferIncrement = decelerationBufferIncrement;
	}

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnStepperQuery(key, bSuccess,
								state,
								bEnabled,
								bForward,
								locatorIndex,
								locatorLineNumberStart,
								locatorLineNumberTerminal,
								homeOffset,
								lowClks,
								highClks,
								accelerationBuffer,
								accelerationBufferDecrement,
								decelerationBuffer,
								decelerationBufferIncrement);
	}
}

void ConsoleOperator::OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput)
{
	Poco::ScopedLock<Poco::Mutex> lowerLock(_lowerMutex);

	if(_cmdKey == InvalidCommandId) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnLocatorQuery unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnLocatorQuery finished");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandId;

	for(auto it=_observerPtrArray.begin(); it!=_observerPtrArray.end(); it++) {
		(*it)->OnLocatorQuery(key, bSuccess, lowInput);
	}
}


void ConsoleOperator::AddObserver(IResponseReceiver * pObserver)
{
	if(pObserver != nullptr) {
		_observerPtrArray.push_back(pObserver);
	}
}


void ConsoleOperator::runTask()
{
	while(1)
	{
		if(isCancelled())
		{
			break;
		}
		else
		{
			std::string cmd;

			char c = getchar();
			_input.push_back(c);

			cmd = getConsoleCommand();

			if(!cmd.empty()) {
				auto success = runConsoleCommand(cmd);
				if(!success) {
					showHelp();
				}
			}
		}
	}

	pLogger->LogInfo("ConsoleOperator::runTask exits");
}
