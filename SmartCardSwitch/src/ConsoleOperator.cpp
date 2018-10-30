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
#include "CommandFactory.h"
#include "ReplyTranslator.h"
#include "Command.h"

extern Logger * pLogger;

ConsoleOperator::ConsoleOperator(): Task("ConsoleOperator")
{
	_userCommand.state = UserCommand::CommandState::IDLE;
	_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::UNKNOWN;
	_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::UNKNOWN;
	_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::UNKNOWN;
	for(int i=0; i<BDC_AMOUNT; i++) {
		_userCommand.resultBdcStatus[i] = UserCommand::BdcStatus::UNKNOWN;
	}

	_pDeviceAccessor = nullptr;

	_pKeyboardObj = new Keyboard(this);
	_keyboardThread.start(*_pKeyboardObj);
}

void ConsoleOperator::SetDevice(DeviceAccessor * pDeviceAccessor)
{
	_pDeviceAccessor = pDeviceAccessor;
}

void ConsoleOperator::OnFeedback(const std::string& feedback)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	//append to end
	_feedbacks.push_back(feedback);
	std::cout << "Feedback: " << feedback << '\n';
	_event.set();
}

void ConsoleOperator::runTask()
{
	while(1)
	{
		if(isCancelled())
		{
			//terminate the keyboard thread.
			_pKeyboardObj->Terminate();
			try
			{
				_keyboardThread.tryJoin(1000);
			}
			catch(...)
			{
				pLogger->LogError("ConsoleOperator::runTask KeyboardThread isn't terminated");
			}

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
				processInput();
				processFeedbacks();
			}
		}
	}

	pLogger->LogInfo("ConsoleOperator::runTask exited");
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
	std::cout << "StepperRun: ----------------------- "<< "72" << "\r\n";
	std::cout << "StepperConfigHome:----------------- "<< "73 stepperIndex locatorIndex lineNumberStart lineNumberTerminal" << "\r\n";
	std::cout << "StepperMove:----------------------- "<< "74 stepperIndex forward stepAmount" << "\r\n";
	std::cout << "StepperQuery: --------------------- "<< "75 stepperIndex" << "\r\n";
	std::cout << "LocatorQuery:---------------------- "<< "90 locatorIndex" << "\r\n";
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
		std::cout << "ConsoleOperator::processInput invalid command: " << command << "\r\n";
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
		std::cout<<"Command: "<<command<<"\r\n";
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

	//create command
	std::shared_ptr<DeviceCommand> cmdPtr (nullptr);
	switch(d0)
	{
		case UserCommand::Type::DevicesGet:
		{
			cmdPtr = CommandFactory::DevicesGet();
			if(cmdPtr == nullptr) {
				pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::DevicesGet");
			}
			else {
				_userCommand.resultDevices.clear();
			}
		}
		break;

		case UserCommand::Type::DeviceConnect:
		{
			auto& devices = _userCommand.resultDevices;

			if(devices.empty()) {
				pLogger->LogError("ConsoleOperator::processInput no device to connect");
			}
			else
			{
				int deviceNumber = d1;

				if(deviceNumber >= devices.size()) {
					pLogger->LogError("ConsoleOperator::processInput wrong device number in command: " + std::to_string(deviceNumber));
				}
				else
				{
					std::string deviceName = devices[deviceNumber];
					cmdPtr = CommandFactory::DeviceConnect(deviceName);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::DeviceConnect");
					}
					else {
						_userCommand.resultConnectedDeviceName.clear();
					}
				}
			}
		}
		break;

		case UserCommand::Type::DeviceQueryPower:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::DeviceQueryPower();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::DeviceQueryPower");
				}
			}
		}
		break;

		case UserCommand::Type::DeviceQueryFuse:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::DeviceQueryFuse();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::DeviceQueryFuse");
				}
			}
		}
		break;

		case UserCommand::Type::BdcsPowerOn:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::BdcsPowerOn();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcsPowerOn");
				}
			}
		}
		break;

		case UserCommand::Type::BdcsPowerOff:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::BdcsPowerOff();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcsPowerOff");
				}
			}
		}
		break;

		case UserCommand::Type::BdcsQueryPower:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::BdcsQueryPower();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcsQueryPower");
				}
			}
		}
		break;

		case UserCommand::Type::BdcCoast:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					//set the intial state to BREAK.
					cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::COAST);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcOperation");
					}
				}
			}
		}
		break;

		case UserCommand::Type::BdcReverse:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					//set the intial state to BREAK.
					cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::REVERSE);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcOperation");
					}
				}
			}
		}
		break;

		case UserCommand::Type::BdcForward:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					//set the intial state to BREAK.
					cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::FORWARD);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcOperation");
					}
				}
			}
		}
		break;

		case UserCommand::Type::BdcBreak:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					//set the intial state to BREAK.
					cmdPtr = CommandFactory::BdcOperation(index, CommandBdcOperation::BdcMode::BREAK, CommandBdcOperation::BdcMode::BREAK);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcOperation");
					}
				}
			}
		}
		break;

		case UserCommand::Type::BdcQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= BDC_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid BDC index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::BdcQuery(index);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::BdcQuery");
					}
				}
			}
		}
		break;

		case UserCommand::Type::SteppersPowerOn:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::SteppersPowerOn();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::SteppersPowerOn");
				}
			}
		}
		break;

		case UserCommand::Type::SteppersPowerOff:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::SteppersPowerOff();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::SteppersPowerOff");
				}
			}
		}
		break;

		case UserCommand::Type::SteppersQueryPower:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::SteppersQueryPower();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::SteppersQueryPower");
				}
			}
		}
		break;

		case UserCommand::Type::StepperQueryResolution:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				cmdPtr = CommandFactory::StepperQueryResolution();
				if(cmdPtr == nullptr) {
					pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperQueryResolution");
				}
			}
		}
		break;

		case UserCommand::Type::StepperConfigStep:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int lowClks = d2;
				unsigned int highClks = d3;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperConfigStep(index, lowClks, highClks);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperConfigStep");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperAccelerationBuffer:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperAccelerationBuffer(index, value);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperAccelerationBuffer");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperAccelerationBufferDecrement:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperAccelerationBufferDecrement(index, value);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperAccelerationBufferDecrement");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperDecelerationBuffer:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperDecelerationBuffer(index, value);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperDecelerationBuffer");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperDecelerationBufferIncrement:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperDecelerationBufferIncrement(index, value);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperDecelerationBufferIncrement");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperEnable:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				bool enable = (d2 != 0);

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperEnable(index, enable);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperEnable");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperForward:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				bool forward = (d2 != 0);

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperForward(index, forward);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperForward");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperSteps:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int value = d2;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperSteps(index, value);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperSteps");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperRun:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int initialPos = d2;
				unsigned int finalPos = d3;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperRun(index, initialPos, finalPos);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperRun");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperConfigHome:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;
				unsigned int locatorIndex = d2;
				unsigned int lineNumberStart = d3;
				unsigned int lineNumberTerminal = d4;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else if((lineNumberStart < LOCATOR_LINE_NUMBER_MIN) || (lineNumberStart > LOCATOR_LINE_NUMBER_MAX)) {
					pLogger->LogError("ConsoleOperator::processInput lineNumberStart is out of range: " + std::to_string(lineNumberStart));
				}
				else if((lineNumberTerminal < LOCATOR_LINE_NUMBER_MIN) || (lineNumberTerminal > LOCATOR_LINE_NUMBER_MAX)) {
					pLogger->LogError("ConsoleOperator::processInput lineNumberStart is out of range: " + std::to_string(lineNumberTerminal));
				}
				else if(lineNumberTerminal == lineNumberStart) {
					pLogger->LogError("ConsoleOperator::processInput lineNumberStart is same as lineNumberTerminal");
				}
				else
				{
					cmdPtr = CommandFactory::StepperConfigHome(index, locatorIndex, lineNumberStart, lineNumberTerminal);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperConfigHome");
					}
				}
			}
		}
		break;

		case UserCommand::Type::StepperQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= STEPPER_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::StepperQuery(index);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::StepperQuery");
					}
				}
			}
		}
		break;

		case UserCommand::Type::LocatorQuery:
		{
			if(_userCommand.resultConnectedDeviceName.empty()) {
				pLogger->LogError("ConsoleOperator::processInput hasn't connected to any device");
			}
			else {
				unsigned int index = d1;

				if(index >= LOCATOR_AMOUNT) {
					pLogger->LogError("ConsoleOperator::processInput invalid stepper index: " + std::to_string(d1));
				}
				else
				{
					cmdPtr = CommandFactory::LocatorQuery(index);
					if(cmdPtr == nullptr) {
						pLogger->LogError("ConsoleOperator::processInput empty ptr returned from CommandFactory::LocatorQuery");
					}
				}
			}
		}
		break;

		default:
		{

			pLogger->LogError("ConsoleOperator::processInput unknown command: " + command);
		}
		break;
	}

	if(cmdPtr == nullptr) {
		showHelp();
	}
	else
	{
		//set common userCommand attribute
		_userCommand.jsonCommandString = cmdPtr->ToJsonCommandString();
		_userCommand.commandKey = cmdPtr->CommandKey();
		_userCommand.commandId = cmdPtr->CommandId();
		_userCommand.expectedResult = cmdPtr->GetFinalState();

		//send out command
		_pDeviceAccessor->SendCommand(_userCommand.jsonCommandString);
		_userCommand.state = UserCommand::CommandState::COMMAND_SENT;
	}
}

bool ConsoleOperator::isCorrespondingReply(const std::string& commandKey, unsigned short commandId)
{
	bool bCorrespondingReply = false;

	if(_userCommand.state != UserCommand::CommandState::COMMAND_SENT) {
		pLogger->LogError("ConsoleOperator::isCorrespondingReply obsolete reply");
	}
	else if(_userCommand.commandKey != commandKey) {
		pLogger->LogError("ConsoleOperator::isCorrespondingReply command key doesn't match, original: '" + _userCommand.commandKey + "', key in reply: '" + commandKey + "'");
	}
	else if((_userCommand.commandId & 0xffff) != commandId) {
		pLogger->LogError("ConsoleOperator::isCorrespondingReply commandId doesn't match, original: " + std::to_string(_userCommand.commandId) + ", cmdId in reply: " + std::to_string(commandId));
	}
	else {
		bCorrespondingReply = true;
	}

	return bCorrespondingReply;
}

void ConsoleOperator::onFeedbackDevicesGet(std::shared_ptr<ReplyTranslator::ReplyDevicesGet> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	//update device list
	_userCommand.resultDevices.clear();
	if(replyPtr->devices.empty()) {
		pLogger->LogError("ConsoleOperator::onFeedbackDevicesGet no devices in: " + replyPtr->originalString);
	}
	else {
		int number = 0;
		for(auto it=replyPtr->devices.begin(); it!=replyPtr->devices.end(); it++, number++) {
			_userCommand.resultDevices.push_back(*it);
			pLogger->LogInfo("ConsoleOperator::onFeedbackDevicesGet device: " + std::to_string(number) + ". " + *it);
			std::cout << "ConsoleOperator::onFeedbackDevicesGet device: " << number << ". " << *it << "\r\n";
		}
	}

	_userCommand.state = UserCommand::CommandState::SUCCEEDED;
}

void ConsoleOperator::onFeedbackDeviceConnect(std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->connected) {
		pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceConnect connected to device: " + replyPtr->deviceName);
		_userCommand.resultConnectedDeviceName = replyPtr->deviceName;
		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceConnect couldn't connect to device: " + replyPtr->deviceName + " reason: " + replyPtr->reason);
		_userCommand.resultConnectedDeviceName.clear();
		_userCommand.state = UserCommand::CommandState::FAILED;
	}
}

void ConsoleOperator::onFeedbackDeviceQueryPower(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bPoweredOn) {
			pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceQueryPower device is powered on");
			_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::POWERED_ON;
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceQueryPower device is powered off");
			_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		}

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		//error happened
		pLogger->LogError("ConsoleOperator::onFeedbackDeviceQueryPower unknown device power status due to: " + replyPtr->errorInfo);
		_userCommand.resultDevicePowerStatus = UserCommand::PowerStatus::UNKNOWN;
		_userCommand.state = UserCommand::CommandState::FAILED;
	}
}

void ConsoleOperator::onFeedbackDeviceQueryFuse(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bFuseOn) {
			pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceQueryFuse main fuse is on");
			_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::FUSE_ON;
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackDeviceQueryFuse main fuse is off");
			_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::FUSE_OFF;
		}

		_userCommand.state = UserCommand::CommandState::SUCCEEDED;
	}
	else {
		//error happened
		pLogger->LogError("ConsoleOperator::onFeedbackDeviceQueryFuse unknown main fuse status due to: " + replyPtr->errorInfo);
		_userCommand.resultDeviceFuseStatus = UserCommand::FuseStatus::UNKNOWN;
		_userCommand.state = UserCommand::CommandState::FAILED;
	}
}

void ConsoleOperator::onFeedbackBdcsPowerOn(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		pLogger->LogInfo("ConsoleOperator::onFeedbackBdcsPowerOn bdcs is powered on");
		_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_ON;
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcsPowerOn error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
	}
}

void ConsoleOperator::onFeedbackBdcsPowerOff(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		pLogger->LogInfo("ConsoleOperator::onFeedbackBdcsPowerOff bdcs is powered off");
		_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_OFF;
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcsPowerOff error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
	}
}

void ConsoleOperator::onFeedbackBdcsQueryPower(std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->bPoweredOn) {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcsQueryPower bdcs is powered on");
			_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_ON;
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcsQueryPower bdcs is powered off");
			_userCommand.resultBdcsPowerStatus = UserCommand::PowerStatus::POWERED_OFF;
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcsQueryPower error: " + replyPtr->errorInfo);
		//keep bdcs power status unchanged.
	}
}

void ConsoleOperator::onFeedbackBdcCoast(std::shared_ptr<ReplyTranslator::ReplyBdcCoast> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("ConsoleOperator::onFeedbackBdcCoast index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcCoast index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::COAST;
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcCoast error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}
}

void ConsoleOperator::onFeedbackBdcReverse(std::shared_ptr<ReplyTranslator::ReplyBdcReverse> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("ConsoleOperator::onFeedbackBdcReverse index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcReverse index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::REVERSE;
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcReverse error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}
}

void ConsoleOperator::onFeedbackBdcForward(std::shared_ptr<ReplyTranslator::ReplyBdcForward> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("ConsoleOperator::onFeedbackBdcForward index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcForward index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::FORWARD;
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcForward error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}
}

void ConsoleOperator::onFeedbackBdcBreak(std::shared_ptr<ReplyTranslator::ReplyBdcBreak> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("ConsoleOperator::onFeedbackBdcBreak index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcBreak index: " + std::to_string(replyPtr->index));
			_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::BREAK;
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcBreak error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}
}

void ConsoleOperator::onFeedbackBdcQuery(std::shared_ptr<ReplyTranslator::ReplyBdcQuery> replyPtr)
{
	if(!isCorrespondingReply(replyPtr->commandKey, replyPtr->commandId)) {
		return;
	}

	if(replyPtr->errorInfo.empty()) {
		//no error
		if(replyPtr->index >= BDC_AMOUNT) {
			pLogger->LogError("ConsoleOperator::onFeedbackBdcQuery index out of range in bdc coast: " + std::to_string(replyPtr->index));
		}
		else {
			pLogger->LogInfo("ConsoleOperator::onFeedbackBdcQuery index: " + std::to_string(replyPtr->index) + " mode: " + std::to_string(replyPtr->index));

			switch(replyPtr->mode)
			{
				case ReplyTranslator::ReplyBdcQuery::BdcMode::COAST:
					_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::COAST;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::REVERSE:
					_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::REVERSE;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::FORWARD:
					_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::FORWARD;
					break;
				case ReplyTranslator::ReplyBdcQuery::BdcMode::BREAK:
					_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::BREAK;
					break;
				default:
					pLogger->LogError("ConsoleOperator::onFeedbackBdcQuery unknown bdc status");
					_userCommand.resultBdcStatus[replyPtr->index] = UserCommand::BdcStatus::UNKNOWN;
					break;
			}
		}
	}
	else {
		pLogger->LogError("ConsoleOperator::onFeedbackBdcQuery error: " + replyPtr->errorInfo + " index: " + std::to_string(replyPtr->index));
	}
}


void ConsoleOperator::processFeedbacks()
{
	if(_feedbacks.empty()) {
		return;
	}
	for(;;)
	{
		if(_feedbacks.empty()) {
			break;
		}

		//retrieve feed back from beginning of _feedbacks
		std::string feedback = _feedbacks.front();
		_feedbacks.pop_front();
		pLogger->LogInfo("ConsoleOperator::processFeedbacks dispose feedback: " + feedback);

		//create a ReplyTranslater object.
		ReplyTranslator translator(feedback);
		auto replyType = translator.Type();

		switch(replyType)
		{
			case ReplyTranslator::ReplyType::DevicesGet:
			{
				auto replyPtr = translator.ToDevicesGet();
				onFeedbackDevicesGet(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceConnect:
			{
				auto replyPtr = translator.ToDeviceConnect();
				onFeedbackDeviceConnect(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceQueryPower:
			{
				auto replyPtr = translator.ToDeviceQueryPower();
				onFeedbackDeviceQueryPower(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::DeviceQueryFuse:
			{
				auto replyPtr = translator.ToDeviceQueryFuse();
				onFeedbackDeviceQueryFuse(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsPowerOn:
			{
				auto replyPtr = translator.ToBdcsPowerOn();
				onFeedbackBdcsPowerOn(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsPowerOff:
			{
				auto replyPtr = translator.ToBdcsPowerOff();
				onFeedbackBdcsPowerOff(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcsQueryPower:
			{
				auto replyPtr = translator.ToBdcsQueryPower();
				onFeedbackBdcsQueryPower(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcCoast:
			{
				auto replyPtr = translator.ToBdcCoast();
				onFeedbackBdcCoast(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcReverse:
			{
				auto replyPtr = translator.ToBdcReverse();
				onFeedbackBdcReverse(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcForward:
			{
				auto replyPtr = translator.ToBdcCoast();
				onFeedbackBdcCoast(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcBreak:
			{
				auto replyPtr = translator.ToBdcBreak();
				onFeedbackBdcBreak(replyPtr);
			}
			break;

			case ReplyTranslator::ReplyType::BdcQuery:
			{
				auto replyPtr = translator.ToBdcQuery();
				onFeedbackBdcQuery(replyPtr);
			}
			break;

			default:
			{
				pLogger->LogError("ConsoleOperator::processFeedbacks unknown feedback: " + feedback);
			}
			break;
		}
	}
}

void ConsoleOperator::OnKeypressing(char key)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_input.push_back(key);
	_event.set();
}


ConsoleOperator::Keyboard::Keyboard(ConsoleOperator * pReceiver)
{
	_terminate = false;
	_pReceiver = pReceiver;
}

void ConsoleOperator::Keyboard::Terminate()
{
	_terminate = true;
}

void ConsoleOperator::Keyboard::run()
{
	while(!_terminate)
	{
		std::string str;
		char buffer[64];

		char c = getchar();

		sprintf(buffer, "0x%02d", c);
		str += buffer;
		if((c >= ' ') && (c <= '~')) {
			sprintf(buffer, " '%c'", c);
			str += buffer;
		}
		pLogger->LogDebug("ConsoleOperator::Keyboard::run received: " + str);
		_pReceiver->OnKeypressing(c);
	}
	pLogger->LogInfo("ConsoleOperator::Keyboard::run exited");
}


