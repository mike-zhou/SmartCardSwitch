/*
 * CommandTranslater.cpp
 *
 *  Created on: Oct 9, 2018
 *      Author: user1
 */

#include "CommandTranslater.h"

CommandTranslator::CommandTranslator(std::string jsonCmd)
{
	this->jsonCmd = jsonCmd;
	type = CommandType::Invalid;
}

std::string CommandTranslator::JsonCommand()
{
	return jsonCmd;
}

CommandType CommandTranslator::Type()
{
	bool exceptionOccur = false;
	std::string command;

	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			command = objectPtr->getValue<std::string>("command");
			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::CommandType invalid command in " + jsonCmd);
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::CommandType no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
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
		type = CommandType::Invalid;
	}
	else
	{
		if(command == strCommandDevicesGet) {
			type = CommandType::DevicesGet;
		}
		else if(command == strCommandDeviceConnect) {
			type = CommandType::DeviceConnect;
		}
		else if(command == strCommandBdcsPowerOn) {
			type = CommandType::BdcsPowerOn;
		}
		else if(command == strCommandBdcsPowerOff) {
			type = CommandType::BdcsPowerOff;
		}
		else if(command == strCommandBdcsQueryPower) {
			type = CommandType::BdcsQueryPower;
		}
		else if(command == strCommandBdcCoast) {
			type = CommandType::BdcCoast;
		}
		else if(command == strCommandBdcReverse) {
			type = CommandType::BdcReverse;
		}
		else if(command == strCommandBdcForward) {
			type = CommandType::BdcForward;
		}
		else if(command == strCommandBdcBreak) {
			type = CommandType::BdcBreak;
		}
		else if(command == strCommandBdcQuery) {
			type = CommandType::BdcQuery;
		}
		else if(command == strCommandSteppersPowerOn) {
			type = CommandType::SteppersPowerOn;
		}
		else if(command == strCommandSteppersPowerOff) {
			type = CommandType::SteppersPowerOff;
		}
		else if(command == strCommandSteppersQueryPower) {
			type = CommandType::SteppersQueryPower;
		}
		else if(command == strCommandStepperQueryResolution) {
			type = CommandType::SteppersQueryPower;
		}
		else if(command == strCommandStepperConfigStep) {
			type = CommandType::StepperConfigStep;
		}
		else if(command == strCommandStepperAccelerationBuffer) {
			type = CommandType::StepperAccelerationBuffer;
		}
		else if(command == strCommandStepperAccelerationBufferDecrement) {
			type = CommandType::StepperAccelerationBufferDecrement;
		}
		else if(command == strCommandStepperAccelerationBufferDecrement) {
			type = CommandType::StepperAccelerationBufferDecrement;
		}
		else if(command == strCommandStepperDecelerationBuffer) {
			type = CommandType::StepperDecelerationBuffer;
		}
		else if(command == strCommandStepperDecelerationBufferIncrement) {
			type = CommandType::StepperDecelerationBufferIncrement;
		}
		else if(command == strCommandStepperEnable) {
			type = CommandType::StepperEnable;
		}
		else if(command == strCommandStepperForward) {
			type = CommandType::StepperForward;
		}
		else if(command == strCommandStepperSteps) {
			type = CommandType::StepperSteps;
		}
		else if(command == strCommandStepperRun) {
			type = CommandType::StepperRun;
		}
		else if(command == strCommandStepperConfigHome) {
			type = CommandType::StepperConfigHome;
		}
		else if(command == strCommandStepperQuery) {
			type = CommandType::StepperQuery;
		}
		else if(command == strCommandLocatorQuery) {
			type = CommandType::LocatorQuery;
		}
		else {
			pLogger->LogError("CommandTranslator::CommandType unknown command in " + jsonCmd);
			type = CommandType::Invalid;
		}
	}

	return type;
}

std::shared_ptr<CommandDevicesGet> CommandTranslator::GetCommandDevicesGet()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDevicesGet invalid command in " + jsonCmd);
			}
			else if(command != strCommandDevicesGet) {
				pLogger->LogError("CommandTranslator::GetCommandDevicesGet wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandDevicesGet> p(new CommandDevicesGet);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDevicesGet no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDevicesGet exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDevicesGet unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandDeviceConnect> CommandTranslator::GetCommandDeviceConnect()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceConnect invalid command in " + jsonCmd);
			}
			else if(command != strCommandDeviceConnect) {
				pLogger->LogError("CommandTranslator::GetCommandDeviceConnect wrong command in " + jsonCmd);
			}
			else
			{
				std::string deviceName = objectPtr->getValue<std::string>("device");
				std::shared_ptr<CommandDeviceConnect> p(new CommandDeviceConnect(deviceName));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandDeviceConnect no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceConnect exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandDeviceConnect unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsPowerOn> CommandTranslator::GetCommandBdcsPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcsPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsPowerOn> p(new CommandBdcsPowerOn);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOn unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsPowerOff> CommandTranslator::GetCommandBdcsPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcsPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsPowerOff> p(new CommandBdcsPowerOff);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsPowerOff unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcsQueryPower> CommandTranslator::GetCommandBdcsQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcsQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandBdcsQueryPower> p(new CommandBdcsQueryPower);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcsQueryPower unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcCoast> CommandTranslator::GetCommandBdcCoast()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcCoast invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcCoast) {
				pLogger->LogError("CommandTranslator::GetCommandBdcCoast wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcCoast> p(new CommandBdcCoast(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcCoast no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcCoast exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcCoast unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcReverse> CommandTranslator::GetCommandBdcReverse()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcReverse invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcReverse) {
				pLogger->LogError("CommandTranslator::GetCommandBdcReverse wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcReverse> p(new CommandBdcReverse(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcReverse no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcReverse exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcReverse unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcForward> CommandTranslator::GetCommandBdcForward()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcForward invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcForward) {
				pLogger->LogError("CommandTranslator::GetCommandBdcForward wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcForward> p(new CommandBdcForward(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcForward no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcForward exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcForward unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcBreak> CommandTranslator::GetCommandBdcBreak()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcBreak invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcBreak) {
				pLogger->LogError("CommandTranslator::GetCommandBdcBreak wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcBreak> p(new CommandBdcBreak(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcBreak no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcBreak exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcBreak unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandBdcQuery> CommandTranslator::GetCommandBdcQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandBdcQuery invalid command in " + jsonCmd);
			}
			else if(command != strCommandBdcQuery) {
				pLogger->LogError("CommandTranslator::GetCommandBdcQuery wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				std::shared_ptr<CommandBdcQuery> p(new CommandBdcQuery(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandBdcQuery no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcQuery exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandBdcQuery unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersPowerOn> CommandTranslator::GetCommandSteppersPowerOn()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn invalid command in " + jsonCmd);
			}
			else if(command != strCommandSteppersPowerOn) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersPowerOn> p(new CommandSteppersPowerOn);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOn unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersPowerOff> CommandTranslator::GetCommandSteppersPowerOff()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff invalid command in " + jsonCmd);
			}
			else if(command != strCommandSteppersPowerOff) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersPowerOff> p(new CommandSteppersPowerOff);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersPowerOff unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandSteppersQueryPower> CommandTranslator::GetCommandSteppersQueryPower()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower invalid command in " + jsonCmd);
			}
			else if(command != strCommandSteppersQueryPower) {
				pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandSteppersQueryPower> p(new CommandSteppersQueryPower);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandSteppersQueryPower unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperQueryResolution> CommandTranslator::GetCommandStepperQueryResolution()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperQueryResolution) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandStepperQueryResolution> p(new CommandStepperQueryResolution);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQueryResolution unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperConfigStep> CommandTranslator::GetCommandStepperConfigStep()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperConfigStep) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int lowClks = objectPtr->getValue<int>("lowClks");
				int highClks = objectPtr->getValue<int>("highClks");

				std::shared_ptr<CommandStepperConfigStep> p(new CommandStepperConfigStep(index, lowClks, highClks));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigStep unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperAccelerationBuffer> CommandTranslator::GetCommandAccelerationBuffer()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperAccelerationBuffer) {
				pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperAccelerationBuffer> p(new CommandStepperAccelerationBuffer(index, value));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandAccelerationBuffer unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperAccelerationBufferDecrement> CommandTranslator::GetCommandStepperAccelerationBufferDecrement()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperAccelerationBufferDecrement) {
				pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperAccelerationBufferDecrement> p(new CommandStepperAccelerationBufferDecrement(index, value));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperAccelerationBufferDecrement unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperDecelerationBuffer> CommandTranslator::GetCommandStepperDecelerationBuffer()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperDecelerationBuffer) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperDecelerationBuffer> p(new CommandStepperDecelerationBuffer(index, value));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBuffer unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperDecelerationBufferIncrement> CommandTranslator::GetCommandStepperDecelerationBufferIncrement()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperDecelerationBufferIncrement) {
				pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperDecelerationBufferIncrement> p(new CommandStepperDecelerationBufferIncrement(index, value));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperDecelerationBufferIncrement unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperEnable> CommandTranslator::GetCommandStepperEnable()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperEnable invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperEnable) {
				pLogger->LogError("CommandTranslator::GetCommandStepperEnable wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				bool enable = objectPtr->getValue<bool>("enable");

				std::shared_ptr<CommandStepperEnable> p(new CommandStepperEnable(index, enable));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperEnable no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperEnable exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperEnable unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperForward> CommandTranslator::GetCommandStepperForward()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperForward invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperForward) {
				pLogger->LogError("CommandTranslator::GetCommandStepperForward wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				bool forward = objectPtr->getValue<bool>("forward");

				std::shared_ptr<CommandStepperForward> p(new CommandStepperForward(index, forward));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperForward no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperForward exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperForward unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperSteps> CommandTranslator::GetCommandStepperSteps()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSteps invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperSteps) {
				pLogger->LogError("CommandTranslator::GetCommandStepperSteps wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");
				int value = objectPtr->getValue<int>("value");

				std::shared_ptr<CommandStepperSteps> p(new CommandStepperSteps(index, value));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperSteps no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSteps exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperSteps unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperRun> CommandTranslator::GetCommandStepperRun()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperRun invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperRun) {
				pLogger->LogError("CommandTranslator::GetCommandStepperRun wrong command in " + jsonCmd);
			}
			else
			{
				std::shared_ptr<CommandStepperRun> p(new CommandStepperRun);
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperRun no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperRun exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperRun unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperConfigHome> CommandTranslator::GetCommandStepperConfigHome()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperConfigHome) {
				pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome wrong command in " + jsonCmd);
			}
			else
			{
				int stepperIndex = objectPtr->getValue<int>("index");
				int locatorIndex = objectPtr->getValue<int>("locatorIndex");
				int lineNumberStart = objectPtr->getValue<int>("lineNumberStart");
				int lineNumberTerminal = objectPtr->getValue<int>("lineNumberTerminal");

				std::shared_ptr<CommandStepperConfigHome> p(new CommandStepperConfigHome(stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperConfigHome unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandStepperQuery> CommandTranslator::GetCommandStepperQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQuery invalid command in " + jsonCmd);
			}
			else if(command != strCommandStepperQuery) {
				pLogger->LogError("CommandTranslator::GetCommandStepperQuery wrong command in " + jsonCmd);
			}
			else
			{
				int stepperIndex = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandStepperQuery> p(new CommandStepperQuery(stepperIndex));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandStepperQuery no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQuery exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandStepperQuery unknown exception in " + jsonCmd);
	}

	return nullptr;
}

std::shared_ptr<CommandLocatorQuery> CommandTranslator::GetCommandLocatorQuery()
{
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(jsonCmd);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command = objectPtr->getValue<std::string>("command");

			if(command.size() < 1) {
				pLogger->LogError("CommandTranslator::GetCommandLocatorQuery invalid command in " + jsonCmd);
			}
			else if(command != strCommandLocatorQuery) {
				pLogger->LogError("CommandTranslator::GetCommandLocatorQuery wrong command in " + jsonCmd);
			}
			else
			{
				int index = objectPtr->getValue<int>("index");

				std::shared_ptr<CommandLocatorQuery> p(new CommandLocatorQuery(index));
				return p;
			}
		}
		else
		{
			pLogger->LogError("CommandTranslator::GetCommandLocatorQuery no command in " + jsonCmd);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("CommandTranslator::GetCommandLocatorQuery exception occurs: " + e.displayText() + " in " + jsonCmd);
	}
	catch(...)
	{
		pLogger->LogError("CommandTranslator::GetCommandLocatorQuery unknown exception in " + jsonCmd);
	}

	return nullptr;
}

