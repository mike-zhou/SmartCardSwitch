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
	_cmdKey = InvalidCommandKey;
	_bCmdFinish = true;
	_bCmdSucceed = false;

	for(int i=0; i<STEPPER_AMOUNT; i++)
	{
		StepperData s;
		_steppers.push_back(s);
	}
}

void ConsoleOperator::showHelp()
{
	std::cout << "\r\n=========== HELP: Command format=========\r\n";
	std::cout << "DevicesGet:------------------------ "<< "0" << "\r\n";
	std::cout << "DeviceConnect:--------------------- "<< "1 deviceNumber" << "\r\n";
	std::cout << "DeviceQueryPower:------------------ "<< "2" << "\r\n";
	std::cout << "DeviceQueryFuse:------------------- "<< "3" << "\r\n";
	std::cout << "OptPowerOn:------------------------ "<< "20" << "\r\n";
	std::cout << "OptPowerOff:----------------------- "<< "21" << "\r\n";
	std::cout << "OptQueryPower:--------------------- "<< "22" << "\r\n";
	std::cout << "DcmPowerOn:------------------------ "<< "30" << "\r\n";
	std::cout << "DcmPowerOff:----------------------- "<< "31" << "\r\n";
	std::cout << "DcmQueryPower:--------------------- "<< "32" << "\r\n";
	std::cout << "BdcsPowerOn:----------------------- "<< "40" << "\r\n";
	std::cout << "BdcsPowerOff:---------------------- "<< "41" << "\r\n";
	std::cout << "BdcsQueryPower:-------------------- "<< "42" << "\r\n";
	std::cout << "BdcCoast:-------------------------- "<< "43 bdcIndex" << "\r\n";
	std::cout << "BdcReverse:------------------------ "<< "44 bdcIndex" << "\r\n";
	std::cout << "BdcForward:------------------------ "<< "45 bdcIndex" << "\r\n";
	std::cout << "BdcBreak:-------------------------- "<< "46 bdcIndex" << "\r\n";
	std::cout << "BdcQuery:-------------------------- "<< "47 bdcIndex" << "\r\n";
	std::cout << "SteppersPowerOn:------------------- "<< "60" << "\r\n";
	std::cout << "SteppersPowerOff:------------------ "<< "61" << "\r\n";
	std::cout << "SteppersQueryPower:---------------- "<< "62" << "\r\n";
	std::cout << "StepperQueryResolution: ----------- "<< "63" << "\r\n";
	std::cout << "StepperConfigStep:----------------- "<< "64 stepperIndex lowClks highClks" << "\r\n";
	std::cout << "StepperAccelerationBuffer:--------- "<< "65 stepperIndex value" << "\r\n";
	std::cout << "StepperAccelerationBufferDecrement: "<< "66 stepperIndex value" << "\r\n";
	std::cout << "StepperDecelerationBuffer:          "<< "67 stepperIndex value" << "\r\n";
	std::cout << "StepperDecelerationBufferIncrement: "<< "68 stepperIndex value" << "\r\n";
	std::cout << "StepperEnable: -------------------- "<< "69 stepperIndex 1/0" << "\r\n";
	std::cout << "StepperForward: ------------------- "<< "70 stepperIndex 1/0" << "\r\n";
	std::cout << "StepperSteps: --------------------- "<< "71 stepperIndex stepAmount" << "\r\n";
	std::cout << "StepperRun: ----------------------- "<< "72 stepperIndex intialPos finalPos" << "\r\n";
	std::cout << "StepperConfigHome:----------------- "<< "73 stepperIndex locatorIndex lineNumberStart lineNumberTerminal" << "\r\n";
	std::cout << "StepperMove:----------------------- "<< "74 stepperIndex forward stepAmount" << "\r\n";
	std::cout << "StepperQuery: --------------------- "<< "75 stepperIndex" << "\r\n";
	std::cout << "StepperForceState: ---------------- "<< "76 stepperIndex state" << "\r\n";
	std::cout << "LocatorQuery:---------------------- "<< "90 locatorIndex" << "\r\n";
	std::cout << "BdcDelay:-------------------------- "<< "200 ms" << "\r\n";
	std::cout << "SaveMovementConfig:---------------- "<< "300" << "\r\n";
	std::cout << "SaveCoordinates:------------------- "<< "350" << "\r\n";
	std::cout << "===============================================\r\n";
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

	for(int i=0; i<STEPPER_AMOUNT; i++)
	{
		auto rc = pMovementConfiguration->GetStepperConfig(i,
															lowClks,
															highClks,
															accelerationBuffer,
															accelerationBufferDecrement,
															decelerationBuffer,
															decelerationBufferIncrement,
															locatorIndex,
															locatorLineNumberStart,
															locatorLineNumberTerminal);
		if(rc == false)
		{
			pLogger->LogError("ConsoleOperator::loadMovementConfig cannot retrieve settings of stepper: " + std::to_string(i));
		}
		else
		{
			//load step
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperConfigStep(i, lowClks, highClks);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config step for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config step for stepper: " + std::to_string(i));
			}

			//load acceleration buffer
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperAccelerationBuffer(i, accelerationBuffer);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config accelerationBuffer for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config accelerationBuffer for stepper: " + std::to_string(i));
			}

			//load acceleration buffer decrement
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperAccelerationBufferDecrement(i, accelerationBufferDecrement);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config accelerationBufferDecrement for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config accelerationBufferDecrement for stepper: " + std::to_string(i));
			}

			//load deceleration buffer
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperDecelerationBuffer(i, decelerationBuffer);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config decelerationBuffer for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config decelerationBuffer for stepper: " + std::to_string(i));
			}

			//load deceleration buffer increment
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperDecelerationBufferIncrement(i, decelerationBufferIncrement);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config decelerationBufferIncrement for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config decelerationBufferIncrement for stepper: " + std::to_string(i));
			}

			//load locator
			_bCmdFinish = false;
			_cmdKey = _pCommandReception->StepperConfigHome(i, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
			//wait for command finish
			for(;;) {
				if(_bCmdFinish) {
					break;
				}
				sleep(10);
			}
			if(_bCmdSucceed) {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig succeeded in config locator for stepper: " + std::to_string(i));
			}
			else {
				pLogger->LogInfo("ConsoleOperator::loadMovementConfig failed in config locator for stepper: " + std::to_string(i));
			}
		}
	}
}

void ConsoleOperator::processInput()
{
	if(_input.size() < 1) {
		return;
	}

	bool bCommandAvailable = false;
	std::string command;

	//find '\n' in the input
	for(auto it=_input.begin(); it!=_input.end(); it++)
	{
		if(*it == '\n') {
			bCommandAvailable = true;
			break;
		}
	}
	if(!bCommandAvailable) {
		return;
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
		return;
	}

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
	try
	{
		pLogger->LogInfo("ConsoleOperator::processInput command: " + command);
		sscanf(command.data(), "%d %d %d %d %d %d %d %d %d %d\n", &d0, &d1, &d2, &d3, &d4, &d5, &d6, &d7, &d8, &d9);
	}
	catch(...)
	{
		std::string e = "ConsoleOperator::processInput exception in parsing input";
		pLogger->LogError(e);
		std::cout << e << "\r\n";
		showHelp();
	}
	std::cout << d0 << " ";
	std::cout << d1 << " ";
	std::cout << d2 << " ";
	std::cout << d3 << " ";
	std::cout << d4 << " ";
	std::cout << d5 << " ";
	std::cout << d6 << " ";
	std::cout << d7 << " ";
	std::cout << d8 << " ";
	std::cout << d9 << "\r\n";

	//run command
	bool bKnownCmd = true;
	switch(d0)
	{
		case Type::DevicesGet:
		{
			_cmdKey = _pCommandReception->DevicesGet();
		}
		break;

		case Type::DeviceConnect:
		{
			if(_devices.empty()) {
				pLogger->LogError("ConsoleOperator::processInput no device to connect");
			}
			else
			{
				int deviceNumber = d1;
				_cmdKey = _pCommandReception->DeviceConnect(deviceNumber);
			}
		}
		break;

		case Type::DeviceQueryPower:
		{
			_cmdKey = _pCommandReception->DeviceQueryPower();
		}
		break;

		case Type::DeviceQueryFuse:
		{
			_cmdKey = _pCommandReception->DeviceQueryFuse();
		}
		break;

		case Type::BdcsPowerOn:
		{
			_cmdKey = _pCommandReception->BdcsPowerOn();
		}
		break;

		case Type::BdcsPowerOff:
		{
			_cmdKey = _pCommandReception->BdcsPowerOff();
		}
		break;

		case Type::BdcsQueryPower:
		{
			_cmdKey = _pCommandReception->BdcsQueryPower();
		}
		break;

		case Type::BdcCoast:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->BdcCoast(index);
		}
		break;

		case Type::BdcReverse:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->BdcReverse(index);
		}
		break;

		case Type::BdcForward:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->BdcForward(index);
		}
		break;

		case Type::BdcBreak:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->BdcBreak(index);
		}
		break;

		case Type::BdcQuery:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->BdcQuery(index);
		}
		break;

		case Type::SteppersPowerOn:
		{
			_cmdKey = _pCommandReception->SteppersPowerOn();
		}
		break;

		case Type::SteppersPowerOff:
		{
			_cmdKey = _pCommandReception->SteppersPowerOff();
		}
		break;

		case Type::SteppersQueryPower:
		{
			_cmdKey = _pCommandReception->SteppersQueryPower();
		}
		break;

		case Type::StepperQueryResolution:
		{
			_cmdKey = _pCommandReception->StepperQueryResolution();
		}
		break;

		case Type::StepperConfigStep:
		{
			unsigned int index = d1;
			unsigned int lowClks = d2;
			unsigned int highClks = d3;
			_cmdKey = _pCommandReception->StepperConfigStep(index, lowClks, highClks);
		}
		break;

		case Type::StepperAccelerationBuffer:
		{
			unsigned int index = d1;
			unsigned int value = d2;
			_cmdKey = _pCommandReception->StepperAccelerationBuffer(index, value);
		}
		break;

		case Type::StepperAccelerationBufferDecrement:
		{
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperAccelerationBufferDecrement(index, value);
		}
		break;

		case Type::StepperDecelerationBuffer:
		{
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperDecelerationBuffer(index, value);
		}
		break;

		case Type::StepperDecelerationBufferIncrement:
		{
			unsigned int index = d1;
			unsigned int value = d2;
			_cmdKey = _pCommandReception->StepperDecelerationBufferIncrement(index, value);
		}
		break;

		case Type::StepperEnable:
		{
			unsigned int index = d1;
			bool enable = (d2 != 0);
			_cmdKey = _pCommandReception->StepperEnable(index, enable);
		}
		break;

		case Type::StepperForward:
		{
			unsigned int index = d1;
			bool forward = (d2 != 0);
			_cmdKey = _pCommandReception->StepperForward(index, forward);
		}
		break;

		case Type::StepperSteps:
		{
			unsigned int index = d1;
			unsigned int value = d2;

			_cmdKey = _pCommandReception->StepperSteps(index, value);
		}
		break;

		case Type::StepperRun:
		{
			unsigned int index = d1;
			unsigned int initialPos = d2;
			unsigned int finalPos = d3;

			_cmdKey = _pCommandReception->StepperRun(index, initialPos, finalPos);
		}
		break;

		case Type::StepperConfigHome:
		{
			unsigned int index = d1;
			unsigned int locatorIndex = d2;
			unsigned int lineNumberStart = d3;
			unsigned int lineNumberTerminal = d4;

			_cmdKey = _pCommandReception->StepperConfigHome(index, locatorIndex, lineNumberStart, lineNumberTerminal);
		}
		break;

		case Type::StepperForceState:
		{
			unsigned int index = d1;
			ICommandReception::StepperState state = d2;

			_cmdKey = _pCommandReception->StepperForceState(index, state);
		}
		break;

		case Type::StepperQuery:
		{
			unsigned int index = d1;

			_cmdKey = _pCommandReception->StepperQuery(index);
		}
		break;

		case Type::LocatorQuery:
		{
			unsigned int index = d1;
			_cmdKey = _pCommandReception->LocatorQuery(index);
		}
		break;

		case Type::BdcDelay:
		{
			_cmdKey = _pCommandReception->BdcDelay(0, d1);
		}
		break;

		case Type::SaveMovementConfig:
		{
			_cmdKey = _pCommandReception->SaveMovementConfig();
		}
		break;

		case Type::LoadMovementConfigStepper:
		{
			loadMovementConfig();
			_cmdKey = InvalidCommandKey;
		}
		break;

		case Type::SaveCoordinates:
		{
			CoordinateStorage::Type type = d1;
			unsigned int index = d2;

			_cmdKey = _pCommandReception->SaveCoordinates(type, index);
		}
		break;

		default:
		{
			bKnownCmd = false;
			pLogger->LogError("ConsoleOperator::processInput unknown command: " + command);
			showHelp();
		}
		break;
	}

	if((bKnownCmd) && (_cmdKey == InvalidCommandKey)) {
		pLogger->LogInfo("ConsoleOperator::processInput no reply will be returned");
	}
}

void ConsoleOperator::OnDevicesGet(CommandKey key, bool bSuccess, const std::vector<std::string>& devices)
{
	if(_cmdKey == InvalidCommandKey) {
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
	_cmdKey = InvalidCommandKey;
	_bCmdSucceed = true;
}

void ConsoleOperator::OnStepperConfigStep(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperConfigStep unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperConfigStep succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
}

void ConsoleOperator::OnStepperAccelerationBuffer(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperAccelerationBuffer unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperAccelerationBuffer succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
}

void ConsoleOperator::OnStepperAccelerationBufferDecrement(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperAccelerationBufferDecrement unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperAccelerationBufferDecrement succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
}

void ConsoleOperator::OnStepperDecelerationBuffer(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperDecelerationBuffer unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperDecelerationBuffer succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
}

void ConsoleOperator::OnStepperDecelerationBufferIncrement(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperDecelerationBufferIncrement unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperDecelerationBufferIncrement succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
}

void ConsoleOperator::OnStepperConfigHome(CommandKey key, bool bSuccess)
{
	if(_cmdKey == InvalidCommandKey) {
		return;
	}
	if(_cmdKey != key) {
		pLogger->LogDebug("ConsoleOperator::OnStepperConfigHome unexpected cmdKey: " + std::to_string(key) + ", expected: " + std::to_string(_cmdKey));
		return;
	}

	pLogger->LogInfo("ConsoleOperator::OnStepperConfigHome succeed");
	_bCmdSucceed = bSuccess;
	_bCmdFinish = true;
	_cmdKey = InvalidCommandKey;
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
			char c = getchar();

			_input.push_back(c);
			processInput();
		}
	}

	pLogger->LogInfo("ConsoleOperator::runTask exits");
}
