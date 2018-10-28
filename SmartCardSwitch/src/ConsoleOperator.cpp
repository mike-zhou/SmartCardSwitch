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

extern Logger * pLogger;

ConsoleOperator::ConsoleOperator(): Task("ConsoleOperator")
{
	_userCommand.state = UserCommand::CommandState::IDLE;
	_userCommand.devicePowerStatus = UserCommand::PowerStatus::UNKNOWN;

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
	std::cout << "DevicesGet: "<<  "0" << "\r\n";
	std::cout << "DeviceConnect: "<< "1 deviceNumber" << "\r\n";
	std::cout << "DeviceQueryPower: "<< "2" << "\r\n";
	std::cout << "OptPowerOn: "<< "20" << "\r\n";
	std::cout << "OptPowerOff: "<< "21" << "\r\n";
	std::cout << "OptQueryPower: "<< "22" << "\r\n";
	std::cout << "DcmPowerOn: "<< "30" << "\r\n";
	std::cout << "DcmPowerOff: "<< "31" << "\r\n";
	std::cout << "DcmQueryPower: "<< "32" << "\r\n";
	std::cout << "BdcsPowerOn: "<< "40" << "\r\n";
	std::cout << "BdcsPowerOff: "<< "41" << "\r\n";
	std::cout << "BdcsQueryPower: "<< "42" << "\r\n";
	std::cout << "BdcCoast: "<< "43 bdcIndex" << "\r\n";
	std::cout << "BdcReverse: "<< "44 bdcIndex" << "\r\n";
	std::cout << "BdcForward: "<< "45 bdcIndex" << "\r\n";
	std::cout << "BdcBreak: "<< "46 bdcIndex" << "\r\n";
	std::cout << "BdcQuery: "<< "47 bdcIndex" << "\r\n";
	std::cout << "SteppersPowerOn: "<< "60" << "\r\n";
	std::cout << "SteppersPowerOff: "<< "61" << "\r\n";
	std::cout << "SteppersQueryPower: "<< "62" << "\r\n";
	std::cout << "StepperQueryResolution: "<< "63" << "\r\n";
	std::cout << "StepperConfigStep: "<< "64 stepperIndex lowClks highClks" << "\r\n";
	std::cout << "StepperAccelerationBuffer: "<< "65 stepperIndex value" << "\r\n";
	std::cout << "StepperAccelerationBufferDecrement: "<< "66 stepperIndex value" << "\r\n";
	std::cout << "StepperDecelerationBuffer: "<< "67 stepperIndex value" << "\r\n";
	std::cout << "StepperDecelerationBufferIncrement: "<< "68 stepperIndex value" << "\r\n";
	std::cout << "StepperEnable: "<< "69 stepperIndex 1/0" << "\r\n";
	std::cout << "StepperForward: "<< "70 stepperIndex 1/0" << "\r\n";
	std::cout << "StepperSteps: "<< "71 stepperIndex stepAmount" << "\r\n";
	std::cout << "StepperRun: "<< "72" << "\r\n";
	std::cout << "StepperConfigHome: "<< "73 stepperIndex locatorIndex lineNumberStart lineNumberTerminal" << "\r\n";
	std::cout << "StepperMove: "<< "74 stepperIndex forward stepAmount" << "\r\n";
	std::cout << "StepperQuery: "<< "75 stepperIndex" << "\r\n";
	std::cout << "LocatorQuery: "<< "90 locatorIndex" << "\r\n";
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
		_userCommand.state = UserCommand::COMMAND_SENT;
	}
}

bool ConsoleOperator::isCorrespondingReply(const std::string& commandKey, unsigned short commandId)
{
	bool bCorrespondingReply = false;

	if(_userCommand.state != UserCommand::State::COMMAND_SENT) {
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

				if(isCorrespondingReply("devices get", replyPtr->commandId))
				{
					//update device list
					_userCommand.resultDevices.clear();
					if(replyPtr->devices.empty()) {
						pLogger->LogError("ConsoleOperator::processFeedbacks no devices in: " + feedback);
					}
					else {
						int number = 0;
						for(auto it=replyPtr->devices.begin(); it!=replyPtr->devices.end(); it++, number++) {
							_userCommand.resultDevices.push_back(*it);
							pLogger->LogInfo("ConsoleOperator::processFeedbacks device: " + std::to_string(number) + ". " + *it);
							std::cout << "ConsoleOperator::processFeedbacks device: " << number << ". " << *it << "\r\n";
						}
					}

					_userCommand.state = UserCommand::CommandState::SUCCEEDED;
				}
			}
			break;

			case ReplyTranslator::ReplyType::DeviceConnect:
			{
				auto replyPtr = translator.ToDeviceConnect();

				if(isCorrespondingReply("device connect", replyPtr->commandId))
				{
					if(replyPtr->connected) {
						pLogger->LogInfo("ConsoleOperator::processFeedbacks connected to device: " + replyPtr->deviceName);
						_userCommand.resultConnectedDeviceName = replyPtr->deviceName;
						_userCommand.state = UserCommand::CommandState::SUCCEEDED;
					}
					else {
						pLogger->LogInfo("ConsoleOperator::processFeedbacks couldn't connect to device: " + replyPtr->deviceName + " reason: " + replyPtr->reason);
						_userCommand.resultConnectedDeviceName.clear();
						_userCommand.state = UserCommand::CommandState::FAILED;
					}
				}
			}
			break;

			case ReplyTranslator::ReplyType::DeviceQueryPower:
			{

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


