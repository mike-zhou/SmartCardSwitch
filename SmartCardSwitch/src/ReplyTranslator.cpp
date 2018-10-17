/*
 * ReplyTranslator.cpp
 *
 *  Created on: Oct 17, 2018
 *      Author: user1
 */

#include <string>
#include <vector>
#include "Poco/Format.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"
#include "ReplyTranslator.h"
#include "Logger.h"

extern Logger * pLogger;

ReplyTranslator::ReplyTranslator(const std::string& reply)
{
	_reply = reply;
	_type = ReplyType::Invalid;
	_devicesGetPtr = nullptr;
	_deviceConnectPtr = nullptr;
	_deviceQueryPowerPtr = nullptr;
	_bdcsPowerOnPtr = nullptr;
	_bdcsPowerOffPtr = nullptr;
	_bdcsQueryPowerPtr = nullptr;
	_bdcCoastPtr = nullptr;
	_bdcReversePtr = nullptr;
	_bdcForwardPtr = nullptr;
	_bdcBreakPtr = nullptr;
	_bdcQueryPtr = nullptr;
	_stepperQueryResolutionPtr = nullptr;
	_stepperConfigStepPtr = nullptr;
	_stepperAccelerationBufferPtr = nullptr;
	_stepperAccelerationBufferDecrementPtr = nullptr;
	_stepperDecelerationBufferPtr = nullptr;
	_stepperDecelerationBufferIncrementPtr = nullptr;
	_stepperEnablePtr = nullptr;
	_stepperConfigHomePtr = nullptr;
	_stepperMovePtr = nullptr;
	_stepperQueryPtr = nullptr;
	_locatorQueryPtr = nullptr;
	_devicePowerPtr = nullptr;
	_deviceConnectionPtr = nullptr;
	_proxyConnectionPtr = nullptr;
	_stepperOutOfRangePtr = nullptr;

	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_reply);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			std::string command;

			command = objectPtr->getValue<std::string>("command");
			if(command.size() < 1) {
				pLogger->LogError("ReplyTranslator::ReplyTranslator invalid command in " + _reply);
			}
			else {
				parseReply(objectPtr, command);
			}
		}
		else if(objectPtr->has(std::string("event")))
		{
			std::string event;

			event = objectPtr->getValue<std::string>("event");
			if(event.size() < 1) {
				pLogger->LogError("ReplyTranslator::ReplyTranslator invalid event in " + _reply);
			}
			else {
				parseEvent(objectPtr, event);
			}
		}
		else
		{
			pLogger->LogError("ReplyTranslator::ReplyTranslator no type in " + _reply);
		}

	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("ReplyTranslator::ReplyTranslator exception occurs: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("ReplyTranslator::ReplyTranslator unknown exception");
	}
}

void ReplyTranslator::parseReply(Poco::JSON::Object::Ptr objectPtr, const std::string& command)
{
	Poco::DynamicStruct ds = *objectPtr;

	unsigned long commandId = ds["commandId"];

	if(command == strCommandDevicesGet)
	{
		std::vector<std::string> devices;
		auto size = ds["devices"];

		for(int i=0; i<size; i++) {
			auto device = ds["devices"][i];
			devices.push_back(device);
		}

		std::shared_ptr<ReplyDevicesGet> ptr (new ReplyDevicesGet);
		ptr->commandId = commandId;
		ptr->devices = devices;
		ptr->originalString = _reply;

		_devicesGetPtr = ptr;
	}
	else if(command == strCommandDeviceConnect)
	{

	}
	else if(command == strCommandBdcsPowerOn) {
	}
	else if(command == strCommandBdcsPowerOff) {
	}
	else if(command == strCommandBdcsQueryPower) {
	}
	else if(command == strCommandBdcCoast) {
	}
	else if(command == strCommandBdcReverse) {
	}
	else if(command == strCommandBdcForward) {
	}
	else if(command == strCommandBdcBreak) {
	}
	else if(command == strCommandBdcQuery) {
	}
	else if(command == strCommandSteppersPowerOn) {
	}
	else if(command == strCommandSteppersPowerOff) {
	}
	else if(command == strCommandSteppersQueryPower) {
	}
	else if(command == strCommandStepperQueryResolution) {
	}
	else if(command == strCommandStepperConfigStep) {
	}
	else if(command == strCommandStepperAccelerationBuffer) {
	}
	else if(command == strCommandStepperAccelerationBufferDecrement) {
	}
	else if(command == strCommandStepperDecelerationBuffer) {
	}
	else if(command == strCommandStepperDecelerationBufferIncrement) {
	}
	else if(command == strCommandStepperEnable) {
	}
	else if(command == strCommandStepperForward) {
	}
	else if(command == strCommandStepperSteps) {
	}
	else if(command == strCommandStepperRun) {
	}
	else if(command == strCommandStepperConfigHome) {
	}
	else if(command == strCommandStepperMove) {
	}
	else if(command == strCommandStepperQuery) {
	}
	else if(command == strCommandLocatorQuery) {

	}
	else {

	}

}

void ReplyTranslator::parseEvent(Poco::JSON::Object::Ptr objectPtr, const std::string& event)
{

}

ReplyTranslator::ReplyType ReplyTranslator::Type()
{
	return _type;
}

std::shared_ptr<ReplyTranslator::ReplyDevicesGet> ReplyTranslator::ToDevicesGet()
{
	return _devicesGetPtr;
}

std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> ReplyTranslator::ToDeviceConnect()
{
	return _deviceConnectPtr;
}

std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> ReplyTranslator::ToDeviceQueryPower()
{
	return _deviceQueryPowerPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> ReplyTranslator::ToBdcsPowerOn()
{
	return _bdcsPowerOnPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> ReplyTranslator::ToBdcsPowerOff()
{
	return _bdcsPowerOffPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> ReplyTranslator::ToBdcsQueryPower()
{
	return _bdcsQueryPowerPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcCoast> ReplyTranslator::ToBdcCoast()
{
	return _bdcCoastPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcReverse> ReplyTranslator::ToBdcReverse()
{
	return _bdcReversePtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcForward> ReplyTranslator::ToBdcForward()
{
	return _bdcForwardPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcBreak> ReplyTranslator::ToBdcBreak()
{
	return _bdcBreakPtr;
}

std::shared_ptr<ReplyTranslator::ReplyBdcQuery> ReplyTranslator::ToBdcQuery()
{
	return _bdcQueryPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> ReplyTranslator::ToStepperQueryResolution()
{
	return _stepperQueryResolutionPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> ReplyTranslator::ToStepperConfigStep()
{
	return _stepperConfigStepPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> ReplyTranslator::ToStepperAccelerationBuffer()
{
	return _stepperAccelerationBufferPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> ReplyTranslator::ToStepperAccelerationBufferDecrement()
{
	return _stepperAccelerationBufferDecrementPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> ReplyTranslator::ToStepperDecelerationBuffer()
{
	return _stepperDecelerationBufferPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> ReplyTranslator::ToStepperDecelerationBufferIncrement()
{
	return _stepperDecelerationBufferIncrementPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperEnable> ReplyTranslator::ToStepperEnable()
{
	return _stepperEnablePtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> ReplyTranslator::ToStepperConfigHome()
{
	return _stepperConfigHomePtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperMove> ReplyTranslator::ToStepperMove()
{
	return _stepperMovePtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperQuery> ReplyTranslator::ToStepperQuery()
{
	return _stepperQueryPtr;
}

std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> ReplyTranslator::ToLocatorQuery()
{
	return _locatorQueryPtr;
}

std::shared_ptr<ReplyTranslator::EventDevicePower> ReplyTranslator::ToDevicePower()
{
	return _devicePowerPtr;
}

std::shared_ptr<ReplyTranslator::EventDeviceConnection> ReplyTranslator::ToDeviceConnection()
{
	return _deviceConnectionPtr;
}

std::shared_ptr<ReplyTranslator::EventProxyConnection> ReplyTranslator::ToProxyConnection()
{
	return _proxyConnectionPtr;
}

std::shared_ptr<ReplyTranslator::EventStepperOutOfRange> ReplyTranslator::ToStepperOutOfRange()
{
	return _stepperOutOfRangePtr;
}
