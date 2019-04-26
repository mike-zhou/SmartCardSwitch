/*
 * CommandTranslater.cpp
 *
 *  Created on: Oct 9, 2018
 *      Author: user1
 */

#include "CommandTranslater.h"

CommandTranslator::CommandTranslator(std::string jsonCmd)
{
	this->_jsonCmd = jsonCmd;
	_type = CommandType::Invalid;
}

std::string CommandTranslator::JsonCommand()
{
	return _jsonCmd;
}

CommandType CommandTranslator::Type()
{
	bool exceptionOccur = false;
	std::string command;

	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			command = objectPtr->getValue<std::string>("command");
			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::CommandType invalid command in " + _jsonCmd);
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::CommandType no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		exceptionOccur = true;
		pLogger->LogError("CommandTranslator::CommandType exception occurs: " + e.displayText());
	}
	catch(...)
	{
		exceptionOccur = true;
		pLogger->LogError("CommandTranslator::CommandType unknown exception");
	}

	if(exceptionOccur)
	{
		_type = CommandType::Invalid;
	}
	else
	{
		if(command == strCommandDevicesGet) {
			_type = CommandType::DevicesGet;
		}
		else if(command == strCommandDeviceConnect) {
			_type = CommandType::DeviceConnect;
		}
		else if(command == strCommandDeviceDelay) {
			_type = CommandType::DeviceDelay;
		}
		else if(command == strCommandDeviceQueryPower) {
			_type = CommandType::DeviceQueryPower;
		}
		else if(command == strCommandDeviceQueryFuse) {
			_type = CommandType::DeviceQueryFuse;
		}
		else if(command == strCommandBdcsPowerOn) {
			_type = CommandType::BdcsPowerOn;
		}
		else if(command == strCommandBdcsPowerOff) {
			_type = CommandType::BdcsPowerOff;
		}
		else if(command == strCommandBdcsQueryPower) {
			_type = CommandType::BdcsQueryPower;
		}
		else if(command == strCommandBdcCoast) {
			_type = CommandType::BdcCoast;
		}
		else if(command == strCommandBdcReverse) {
			_type = CommandType::BdcReverse;
		}
		else if(command == strCommandBdcForward) {
			_type = CommandType::BdcForward;
		}
		else if(command == strCommandBdcBreak) {
			_type = CommandType::BdcBreak;
		}
		else if(command == strCommandBdcQuery) {
			_type = CommandType::BdcQuery;
		}
		else if(command == strCommandSteppersPowerOn) {
			_type = CommandType::SteppersPowerOn;
		}
		else if(command == strCommandSteppersPowerOff) {
			_type = CommandType::SteppersPowerOff;
		}
		else if(command == strCommandSteppersQueryPower) {
			_type = CommandType::SteppersQueryPower;
		}
		else if(command == strCommandStepperQueryResolution) {
			_type = CommandType::StepperQueryResolution;
		}
		else if(command == strCommandStepperConfigStep) {
			_type = CommandType::StepperConfigStep;
		}
		else if(command == strCommandStepperAccelerationBuffer) {
			_type = CommandType::StepperAccelerationBuffer;
		}
		else if(command == strCommandStepperAccelerationBufferDecrement) {
			_type = CommandType::StepperAccelerationBufferDecrement;
		}
		else if(command == strCommandStepperAccelerationBufferDecrement) {
			_type = CommandType::StepperAccelerationBufferDecrement;
		}
		else if(command == strCommandStepperDecelerationBuffer) {
			_type = CommandType::StepperDecelerationBuffer;
		}
		else if(command == strCommandStepperDecelerationBufferIncrement) {
			_type = CommandType::StepperDecelerationBufferIncrement;
		}
		else if(command == strCommandStepperEnable) {
			_type = CommandType::StepperEnable;
		}
		else if(command == strCommandStepperForward) {
			_type = CommandType::StepperForward;
		}
		else if(command == strCommandStepperSteps) {
			_type = CommandType::StepperSteps;
		}
		else if(command == strCommandStepperRun) {
			_type = CommandType::StepperRun;
		}
		else if(command == strCommandStepperConfigHome) {
			_type = CommandType::StepperConfigHome;
		}
		else if(command == strCommandStepperQuery) {
			_type = CommandType::StepperQuery;
		}
		else if(command == strCommandStepperSetState) {
			_type = CommandType::StepperSetState;
		}
		else if(command == strCommandLocatorQuery) {
			_type = CommandType::LocatorQuery;
		}
		else if(command == strCommandOptPowerOn) {
			_type = CommandType::OptPowerOn;
		}
		else if(command == strCommandOptPowerOff) {
			_type = CommandType::OptPowerOff;
		}
		else if(command == strCommandOptQueryPower) {
			_type = CommandType::OptQueryPower;
		}
		else if(command == strCommandDcmPowerOn) {
			_type = CommandType::DcmPowerOn;
		}
		else if(command == strCommandDcmPowerOff) {
			_type = CommandType::DcmPowerOff;
		}
		else if(command == strCommandDcmQueryPower) {
			_type = CommandType::DcmQueryPower;
		}
		else if(command == strCommandSolenoidActivate) {
			_type = CommandType::SolenoidActivate;
		}
		else {
			pLogger->LogError("CommandTranslator::CommandType unknown command in " + _jsonCmd);
			_type = CommandType::Invalid;
		}
	}

	return _type;
}

std::shared_ptr<CommandDevicesGet> CommandTranslator::GetCommandDevicesGet()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDevicesGet invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDevicesGet) {
				pLogger->LogError("CommandTranslator::GetCommandDevicesGet wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandDevicesGet> p(new CommandDevicesGet(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDevicesGet no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDevicesGet exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDevicesGet unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDeviceConnect> CommandTranslator::GetCommandDeviceConnect()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceConnect invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDeviceConnect) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceConnect wrong command in " + _jsonCmd);
			}
			else
			{
				std::string deviceName = objectPtr->getValue<std::string>("device");
				std::shared_ptr<CommandDeviceConnect> p(new CommandDeviceConnect(commandId, deviceName));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDeviceConnect no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceConnect exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceConnect unknown exception in " + _jsonCmd);
	}

	return nullptr;
}


std::shared_ptr<CommandDeviceQueryPower> CommandTranslator::GetCommandDeviceQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceQueryPower invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDeviceQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceQueryPower wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandDeviceQueryPower> p(new CommandDeviceQueryPower(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDeviceQueryPower no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceQueryPower exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceQueryPower unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDeviceQueryFuse> CommandTranslator::GetCommandDeviceQueryFuse()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceQueryFuse invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDeviceQueryFuse) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceQueryFuse wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandDeviceQueryFuse> p(new CommandDeviceQueryFuse(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDeviceQueryFuse no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceQueryFuse exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceQueryFuse unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDeviceDelay> CommandTranslator::GetCommandDeviceDelay()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");
			unsigned int clks = objectPtr->getValue<unsigned int>("clks");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceDelay invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDeviceDelay) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceDelay wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandDeviceDelay> p(new CommandDeviceDelay(commandId, clks));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDeviceDelay no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceDelay exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceDelay unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsPowerOn> CommandTranslator::GetCommandBdcsPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcsPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsPowerOn> p(new CommandBdcsPowerOn(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsPowerOff> CommandTranslator::GetCommandBdcsPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcsPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsPowerOff> p(new CommandBdcsPowerOff(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsQueryPower> CommandTranslator::GetCommandBdcsQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcsQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsQueryPower> p(new CommandBdcsQueryPower(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcCoast> CommandTranslator::GetCommandBdcCoast()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcCoast invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcCoast) {
				pLogger->LogError("CommandTranslator::GetCommandBdcCoast wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcCoast> p(new CommandBdcCoast(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcCoast no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcCoast exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcCoast unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcReverse> CommandTranslator::GetCommandBdcReverse()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcReverse invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcReverse) {
				pLogger->LogError("CommandTranslator::GetCommandBdcReverse wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int lowClks = objectPtr->getValue<int>("lowClks");
				int highClks = objectPtr->getValue<int>("highClks");
				int cycles = objectPtr->getValue<int>("cycles");
				std::shared_ptr<CommandBdcReverse> p(new CommandBdcReverse(index, lowClks, highClks, cycles, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcReverse no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcReverse exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcReverse unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcForward> CommandTranslator::GetCommandBdcForward()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcForward invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcForward) {
				pLogger->LogError("CommandTranslator::GetCommandBdcForward wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int lowClks = objectPtr->getValue<int>("lowClks");
				int highClks = objectPtr->getValue<int>("highClks");
				int cycles = objectPtr->getValue<int>("cycles");
				std::shared_ptr<CommandBdcForward> p(new CommandBdcForward(index, lowClks, highClks, cycles, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcForward no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcForward exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcForward unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcBreak> CommandTranslator::GetCommandBdcBreak()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcBreak invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcBreak) {
				pLogger->LogError("CommandTranslator::GetCommandBdcBreak wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcBreak> p(new CommandBdcBreak(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcBreak no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcBreak exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcBreak unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcQuery> CommandTranslator::GetCommandBdcQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcQuery invalid command in " + _jsonCmd);
			}
			else if(command != strCommandBdcQuery) {
				pLogger->LogError("CommandTranslator::GetCommandBdcQuery wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcQuery> p(new CommandBdcQuery(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcQuery no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcQuery exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcQuery unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersPowerOn> CommandTranslator::GetCommandSteppersPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn invalid command in " + _jsonCmd);
			}
			else if(command != strCommandSteppersPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersPowerOn> p(new CommandSteppersPowerOn(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersPowerOff> CommandTranslator::GetCommandSteppersPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff invalid command in " + _jsonCmd);
			}
			else if(command != strCommandSteppersPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersPowerOff> p(new CommandSteppersPowerOff(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersQueryPower> CommandTranslator::GetCommandSteppersQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower invalid command in " + _jsonCmd);
			}
			else if(command != strCommandSteppersQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersQueryPower> p(new CommandSteppersQueryPower(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperQueryResolution> CommandTranslator::GetCommandStepperQueryResolution()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperQueryResolution) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandStepperQueryResolution> p(new CommandStepperQueryResolution(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperConfigStep> CommandTranslator::GetCommandStepperConfigStep()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperConfigStep) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int lowClks = objectPtr->getValue<int>("lowClks");
				int highClks = objectPtr->getValue<int>("highClks");

				std::shared_ptr<CommandStepperConfigStep> p(new CommandStepperConfigStep(index, lowClks, highClks, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperAccelerationBuffer> CommandTranslator::GetCommandStepperAccelerationBuffer()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperAccelerationBuffer) {
				pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperAccelerationBuffer> p(new CommandStepperAccelerationBuffer(index, value, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperAccelerationBufferDecrement> CommandTranslator::GetCommandStepperAccelerationBufferDecrement()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperAccelerationBufferDecrement) {
				pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperAccelerationBufferDecrement> p(new CommandStepperAccelerationBufferDecrement(index, value, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperDecelerationBuffer> CommandTranslator::GetCommandStepperDecelerationBuffer()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperDecelerationBuffer) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperDecelerationBuffer> p(new CommandStepperDecelerationBuffer(index, value,  commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperDecelerationBufferIncrement> CommandTranslator::GetCommandStepperDecelerationBufferIncrement()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperDecelerationBufferIncrement) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperDecelerationBufferIncrement> p(new CommandStepperDecelerationBufferIncrement(index, value, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperEnable> CommandTranslator::GetCommandStepperEnable()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperEnable invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperEnable) {
				pLogger->LogError("CommandTranslator::GetCommandStepperEnable wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				bool enable = objectPtr->getValue<bool>("enable");

				std::shared_ptr<CommandStepperEnable> p(new CommandStepperEnable(index, enable, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperEnable no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperEnable exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperEnable unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperForward> CommandTranslator::GetCommandStepperForward()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperForward invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperForward) {
				pLogger->LogError("CommandTranslator::GetCommandStepperForward wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				bool forward = objectPtr->getValue<bool>("forward");

				std::shared_ptr<CommandStepperForward> p(new CommandStepperForward(index, forward, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperForward no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperForward exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperForward unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperSteps> CommandTranslator::GetCommandStepperSteps()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSteps invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperSteps) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSteps wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperSteps> p(new CommandStepperSteps(index, value, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperSteps no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSteps exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSteps unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperRun> CommandTranslator::GetCommandStepperRun()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperRun invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperRun) {
				pLogger->LogError("CommandTranslator::GetCommandStepperRun wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandStepperRun> p(new CommandStepperRun(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperRun no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperRun exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperRun unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperConfigHome> CommandTranslator::GetCommandStepperConfigHome()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperConfigHome) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome wrong command in " + _jsonCmd);
			}
			else
			{
				int stepperIndex = objectPtr->getValue<int>("index");
				int locatorIndex = objectPtr->getValue<int>("locatorIndex");
				int lineNumberStart = objectPtr->getValue<int>("lineNumberStart");
				int lineNumberTerminal = objectPtr->getValue<int>("lineNumberTerminal");

				std::shared_ptr<CommandStepperConfigHome> p(new CommandStepperConfigHome(stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperQuery> CommandTranslator::GetCommandStepperQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQuery invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperQuery) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQuery wrong command in " + _jsonCmd);
			}
			else
			{
				int stepperIndex = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandStepperQuery> p(new CommandStepperQuery(stepperIndex, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperQuery no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQuery exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQuery unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperSetState> CommandTranslator::GetCommandStepperSetState()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSetState invalid command in " + _jsonCmd);
			}
			else if(command != strCommandStepperSetState) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSetState wrong command in " + _jsonCmd);
			}
			else
			{
				int stepperIndex = objectPtr->getValue<int>("index");
				int state = objectPtr->getValue<int>("state");

				std::shared_ptr<CommandStepperSetState> p(new CommandStepperSetState(stepperIndex, state, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperSetState no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSetState exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSetState unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandLocatorQuery> CommandTranslator::GetCommandLocatorQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandLocatorQuery invalid command in " + _jsonCmd);
			}
			else if(command != strCommandLocatorQuery) {
				pLogger->LogError("CommandTranslator::GetCommandLocatorQuery wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandLocatorQuery> p(new CommandLocatorQuery(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandLocatorQuery no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandLocatorQuery exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandLocatorQuery unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandOptPowerOn> CommandTranslator::GetCommandOptPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandOptPowerOn invalid command in " + _jsonCmd);
			}
			else if(command != strCommandOptPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandOptPowerOn wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandOptPowerOn> p(new CommandOptPowerOn(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandOptPowerOn no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptPowerOn exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptPowerOn unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandOptPowerOff> CommandTranslator::GetCommandOptPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandOptPowerOff invalid command in " + _jsonCmd);
			}
			else if(command != strCommandOptPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandOptPowerOff wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandOptPowerOff> p(new CommandOptPowerOff(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandOptPowerOff no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptPowerOff exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptPowerOff unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandOptQueryPower> CommandTranslator::GetCommandOptQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandOptQueryPower invalid command in " + _jsonCmd);
			}
			else if(command != strCommandOptQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandOptQueryPower wrong command in " + _jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandOptQueryPower> p(new CommandOptQueryPower(commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandOptQueryPower no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptQueryPower exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandOptQueryPower unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDcmPowerOn> CommandTranslator::GetCommandDcmPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDcmPowerOn invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDcmPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandDcmPowerOn wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandDcmPowerOn> p(new CommandDcmPowerOn(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDcmPowerOn no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmPowerOn exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmPowerOn unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDcmPowerOff> CommandTranslator::GetCommandDcmPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDcmPowerOff invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDcmPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandDcmPowerOff wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandDcmPowerOff> p(new CommandDcmPowerOff(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDcmPowerOff no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmPowerOff exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmPowerOff unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDcmQueryPower> CommandTranslator::GetCommandDcmQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDcmQueryPower invalid command in " + _jsonCmd);
			}
			else if(command != strCommandDcmQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandDcmQueryPower wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandDcmQueryPower> p(new CommandDcmQueryPower(index, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDcmQueryPower no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmQueryPower exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDcmQueryPower unknown exception in " + _jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSolenoidActivate> CommandTranslator::GetCommandSolenoidActivate()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");
			unsigned long commandId = objectPtr->getValue<unsigned long>("commandId");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSolenoidActivate invalid command in " + _jsonCmd);
			}
			else if(command != strCommandSolenoidActivate) {
				pLogger->LogError("CommandTranslator::GetCommandSolenoidActivate wrong command in " + _jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				unsigned int lowClks = objectPtr->getValue<unsigned int>("lowClks");
				unsigned int highClks = objectPtr->getValue<unsigned int>("highClks");

				std::shared_ptr<CommandSolenoidActivate> p(new CommandSolenoidActivate(index, lowClks, highClks, commandId));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSolenoidActivate no command in " + _jsonCmd);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSolenoidActivate exception occurs: " + e.displayText() + " in " + _jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSolenoidActivate unknown exception in " + _jsonCmd);
	}

	return nullptr;
}
