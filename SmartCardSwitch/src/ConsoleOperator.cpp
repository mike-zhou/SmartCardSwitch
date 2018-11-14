/*
 * CommandRunner.cpp
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#include "stdio.h"
#include <iostream>

#include "ConsoleOperator.h"
#include "Logger.h"

extern Logger * pLogger;

ConsoleOperator::ConsoleOperator(ICommandReception * pCmdReceiver) : Task("ConsoleOperator")
{
	_pCommandReception = pCmdReceiver;
}

void ConsoleOperator::showHelp()
{
	std::cout << "Command format:\r\n";
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
	std::cout << "LocatorQuery:---------------------- "<< "90 locatorIndex" << "\r\n";
	std::cout << "BdcDelay:-------------------------- "<< "200 ms" << "\r\n";
	std::cout << "SaveMovementConfig:---------------- "<< "300" << "\r\n";
	std::cout << "SaveCoordinates:------------------- "<< "350" << "\r\n";
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
		pLogger->LogError("CommandRunner::processInput invalid command: " + command);
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
		std::string e = "CommandRunner::processInput exception in parsing input";
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

	//create command
	switch(d0)
	{
		case Type::DevicesGet:
		{
			_cmdKey = _pCommandReception->DevicesGet();
		}
		break;

		case Type::DeviceConnect:
		{
			auto& devices = _userCommand.resultDevices;

			if(devices.empty()) {
				pLogger->LogError("CommandRunner::processInput no device to connect");
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
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->BdcCoast(index);
				}
			}
		}
		break;

		case Type::BdcReverse:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->BdcReverse(index);
				}
			}
		}
		break;

		case Type::BdcForward:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->BdcForward(index);
				}
			}
		}
		break;

		case Type::BdcBreak:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->BdcBreak(index);
				}
			}
		}
		break;

		case Type::BdcQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->BdcQuery(index);
				}
			}
		}
		break;

		case Type::SteppersPowerOn:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				_cmdKey = _pCommandReception->SteppersPowerOn();
			}
		}
		break;

		case Type::SteppersPowerOff:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				_cmdKey = _pCommandReception->SteppersPowerOff();
			}
		}
		break;

		case Type::SteppersQueryPower:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				_cmdKey = _pCommandReception->SteppersQueryPower();
			}
		}
		break;

		case Type::StepperQueryResolution:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				_cmdKey = _pCommandReception->StepperQueryResolution();
			}
		}
		break;

		case Type::StepperConfigStep:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int lowClks = d2;
				unsigned int highClks = d3;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperConfigStep(index, lowClks, highClks);
				}
			}
		}
		break;

		case Type::StepperAccelerationBuffer:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperAccelerationBuffer(index, value);
				}
			}
		}
		break;

		case Type::StepperAccelerationBufferDecrement:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperAccelerationBufferDecrement(index, value);
				}
			}
		}
		break;

		case Type::StepperDecelerationBuffer:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperDecelerationBuffer(index, value);
				}
			}
		}
		break;

		case Type::StepperDecelerationBufferIncrement:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperDecelerationBufferIncrement(index, value);
				}
			}
		}
		break;

		case Type::StepperEnable:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				bool enable = (d2 != 0);

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperEnable(index, enable);
				}
			}
		}
		break;

		case Type::StepperForward:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				bool forward = (d2 != 0);

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperForward(index, forward);
				}
			}
		}
		break;

		case Type::StepperSteps:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperSteps(index, value);
				}
			}
		}
		break;

		case Type::StepperRun:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int initialPos = d2;
				unsigned int finalPos = d3;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperRun(index, initialPos, finalPos);
				}
			}
		}
		break;

		case Type::StepperConfigHome:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int locatorIndex = d2;
				unsigned int lineNumberStart = d3;
				unsigned int lineNumberTerminal = d4;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else if((lineNumberStart < LOCATOR_LINE_NUMBER_MIN) || (lineNumberStart > LOCATOR_LINE_NUMBER_MAX)) {
					pLogger->LogError("CommandRunner::processInput lineNumberStart is out of range: " + std::to_string(lineNumberStart));
				}
				else if((lineNumberTerminal < LOCATOR_LINE_NUMBER_MIN) || (lineNumberTerminal > LOCATOR_LINE_NUMBER_MAX)) {
					pLogger->LogError("CommandRunner::processInput lineNumberStart is out of range: " + std::to_string(lineNumberTerminal));
				}
				else if(lineNumberTerminal == lineNumberStart) {
					pLogger->LogError("CommandRunner::processInput lineNumberStart is same as lineNumberTerminal");
				}
				else
				{
					_cmdKey = _pCommandReception->StepperConfigHome(index, locatorIndex, lineNumberStart, lineNumberTerminal);
				}
			}
		}
		break;

		case Type::StepperQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->StepperQuery(index);
				}
			}
		}
		break;

		case Type::LocatorQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("CommandRunner::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= LOCATOR_AMOUNT) {
					pLogger->LogError("CommandRunner::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					_cmdKey = _pCommandReception->LocatorQuery(index);
				}
			}
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

		case Type::SaveCoordinates:
		{
			unsigned int type = d1;
			unsigned int index = d2;

			_cmdKey = _pCommandReception->SaveCoordinates(type, index);
		}
		break;

		default:
		{
			pLogger->LogError("CommandRunner::processInput unknown command: " + command);
			showHelp();
		}
		break;
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
			char c = getchar();

			_input.push_back(c);
			processInput();
		}
	}

	pLogger->LogInfo("ConsoleOperator::runTask exits");
}
