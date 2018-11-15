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
	_stepperForwardPtr = nullptr;
	_stepperStepsPtr = nullptr;
	_stepperRunPtr = nullptr;
	_stepperConfigHomePtr = nullptr;
	_stepperMovePtr = nullptr;
	_stepperQueryPtr = nullptr;
	_stepperSetStatePtr = nullptr;
	_locatorQueryPtr = nullptr;
	_devicePowerPtr = nullptr;
	_deviceConnectionPtr = nullptr;
	_proxyConnectionPtr = nullptr;
	_stepperOutOfRangePtr = nullptr;

	//parse the reply in the constructor
	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_reply);
		Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(objectPtr->has(std::string("command")))
		{
			//dispose reply to command.
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
			//dispose event.
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
	catch(Poco::Exception& e)
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

	//common attributes
	std::string commandKey = ds["command"].toString();
	unsigned short commandId = ds["commandId"];
	std::string errorInfo;
	if(objectPtr->has("error")) {
		errorInfo = ds["error"].toString();
	}

	if(command == strCommandDevicesGet)
	{
		_type = ReplyType::DevicesGet;

		std::vector<std::string> devices;
		auto size = ds["devices"].size();

		for(unsigned int i=0; i<size; i++) {
			auto device = ds["devices"][i].toString();
			devices.push_back(device);
		}

		std::shared_ptr<ReplyDevicesGet> ptr (new ReplyDevicesGet);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->devices = devices;

		_devicesGetPtr = ptr;
	}
	else if(command == strCommandDeviceConnect)
	{
		_type = ReplyType::DeviceConnect;

		std::string deviceName;
		bool connected;
		std::string reason;

		deviceName = ds["deviceName"].toString();
		connected = ds["connected"];
		reason = ds["reason"].toString();

		std::shared_ptr<ReplyDeviceConnect> ptr (new ReplyDeviceConnect);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->deviceName = deviceName;
		ptr->connected = connected;
		ptr->reason = reason;

		_deviceConnectPtr = ptr;
	}
	else if(command == strCommandDeviceQueryPower)
	{
		_type = ReplyType::DeviceQueryPower;

		std::string errorInfo;
		std::string state;

		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}
		else {
			state = ds["state"].toString();
		}

		std::shared_ptr<ReplyDeviceQueryPower> ptr (new ReplyDeviceQueryPower);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			if(state == "powered on") {
				ptr->bPoweredOn = true;
			}
			else if(state == "powered off") {
				ptr->bPoweredOn = false;
			}
			else {
				pLogger->LogError("ReplyTranslator::parseReply unknown device power status: " + state);
				ptr->errorInfo = "unknown device power status";
			}
		}

		_deviceQueryPowerPtr = ptr;
	}
	else if(command == strCommandDeviceQueryFuse)
	{
		_type = ReplyType::DeviceQueryFuse;

		std::string errorInfo;
		std::string state;

		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}
		else {
			state = ds["state"].toString();
		}

		std::shared_ptr<ReplyDeviceQueryFuse> ptr (new ReplyDeviceQueryFuse);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			if(state == "main fuse is on") {
				ptr->bFuseOn = true;
			}
			else if(state == "main fuse is off") {
				ptr->bFuseOn = false;
			}
			else {
				pLogger->LogError("ReplyTranslator::parseReply unknown device fuse status: " + state);
				ptr->errorInfo = "unknown device power status";
			}
		}

		_deviceQueryFusePtr = ptr;
	}
	else if(command == strCommandBdcsPowerOn)
	{
		_type = ReplyType::BdcsPowerOn;

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcsPowerOn> ptr (new ReplyBdcsPowerOn);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;

		_bdcsPowerOnPtr = ptr;
	}
	else if(command == strCommandBdcsPowerOff)
	{
		_type = ReplyType::BdcsPowerOff;

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcsPowerOff> ptr (new ReplyBdcsPowerOff);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;

		_bdcsPowerOffPtr = ptr;
	}
	else if(command == strCommandBdcsQueryPower)
	{
		_type = ReplyType::BdcsQueryPower;

		std::string errorInfo;
		std::string state;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}
		else {
			state = ds["state"].toString();
		}

		std::shared_ptr<ReplyBdcsQueryPower> ptr (new ReplyBdcsQueryPower);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty())
		{
			if(state == "BDCs are powered on") {
				ptr->bPoweredOn = true;
			}
			else if(state == "BDCs are powered off") {
				ptr->bPoweredOn = false;
			}
			else {
				pLogger->LogError("ReplyTranslator::parseReply unknown bdcs power status: " + state);
				ptr->errorInfo = "unknown bdcs power status";
			}
		}

		_bdcsQueryPowerPtr = ptr;
	}
	else if(command == strCommandBdcCoast)
	{
		_type = ReplyType::BdcCoast;

		unsigned int index = ds["index"];

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcCoast> ptr (new ReplyBdcCoast);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->index = index;

		_bdcCoastPtr = ptr;
	}
	else if(command == strCommandBdcReverse)
	{
		_type = ReplyType::BdcReverse;

		unsigned int index = ds["index"];

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcReverse> ptr (new ReplyBdcReverse);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->index = index;

		_bdcReversePtr = ptr;
	}
	else if(command == strCommandBdcForward)
	{
		_type = ReplyType::BdcForward;

		unsigned int index = ds["index"];

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcForward> ptr (new ReplyBdcForward);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->index = index;

		_bdcForwardPtr = ptr;
	}
	else if(command == strCommandBdcBreak)
	{
		_type = ReplyType::BdcBreak;

		unsigned int index = ds["index"];

		std::string errorInfo;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}

		std::shared_ptr<ReplyBdcBreak> ptr (new ReplyBdcBreak);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->index = index;

		_bdcBreakPtr = ptr;
	}
	else if(command == strCommandBdcQuery)
	{
		_type = ReplyType::BdcQuery;

		unsigned int index = ds["index"];

		std::string errorInfo;
		std::string state;
		if(objectPtr->has("error")) {
			errorInfo = ds["error"].toString();
		}
		else {
			state = ds["state"].toString();
		}

		std::shared_ptr<ReplyBdcQuery> ptr (new ReplyBdcQuery);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		ptr->index = index;
		if(errorInfo.empty())
		{
			if(state == "coast") {
				ptr->mode = ReplyBdcQuery::BdcMode::COAST;
			}
			else if(state == "reverse") {
				ptr->mode = ReplyBdcQuery::BdcMode::REVERSE;
			}
			else if(state == "forward") {
				ptr->mode = ReplyBdcQuery::BdcMode::FORWARD;
			}
			else if(state == "break") {
				ptr->mode = ReplyBdcQuery::BdcMode::BREAK;
			}
			else {
				pLogger->LogError("ReplyTranslator::parseReply wrong bdc state: " + state);
				ptr->errorInfo = "wrong bdc status";
			}
		}

		_bdcQueryPtr = ptr;
	}
	else if(command == strCommandSteppersPowerOn)
	{
		_type = ReplyType::SteppersPowerOn;

		std::shared_ptr<ReplySteppersPowerOn> ptr (new ReplySteppersPowerOn);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;

		_steppersPowerOnPtr = ptr;
	}
	else if(command == strCommandSteppersPowerOff)
	{
		_type = ReplyType::SteppersPowerOff;

		std::shared_ptr<ReplySteppersPowerOff> ptr (new ReplySteppersPowerOff);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;

		_steppersPowerOffPtr = ptr;
	}
	else if(command == strCommandSteppersQueryPower)
	{
		_type = ReplyType::SteppersQueryPower;

		std::shared_ptr<ReplySteppersQueryPower> ptr (new ReplySteppersQueryPower);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty())
		{
			//no error
			std::string state = ds["state"].toString();
			if(state == "steppers are powered on") {
				ptr->bPowered = true;
			}
			else if(state == "steppers are powered off") {
				ptr->bPowered = false;
			}
			else {
				pLogger->LogError("ReplyTranslator::parseReply wrong steppers power status: " + state);
				ptr->errorInfo = "unknown steppers power status";
			}
		}

		_steppersQueryPowerPtr = ptr;
	}
	else if(command == strCommandStepperQueryResolution)
	{
		_type = ReplyType::StepperQueryResolution;

		std::shared_ptr<ReplyStepperQueryResolution> ptr (new ReplyStepperQueryResolution);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty())
		{
			//no error
			ptr->resolutionUs = ds["resolution"];
		}

		_stepperQueryResolutionPtr = ptr;
	}
	else if(command == strCommandStepperConfigStep)
	{
		_type = ReplyType::StepperConfigStep;

		std::shared_ptr<ReplyStepperConfigStep> ptr (new ReplyStepperConfigStep);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperConfigStepPtr = ptr;
	}
	else if(command == strCommandStepperAccelerationBuffer)
	{
		_type = ReplyType::StepperAccelerationBuffer;

		std::shared_ptr<ReplyStepperAccelerationBuffer> ptr (new ReplyStepperAccelerationBuffer);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperAccelerationBufferPtr = ptr;
	}
	else if(command == strCommandStepperAccelerationBufferDecrement)
	{
		_type = ReplyType::StepperAccelerationBufferDecrement;

		std::shared_ptr<ReplyStepperAccelerationBufferDecrement> ptr (new ReplyStepperAccelerationBufferDecrement);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperAccelerationBufferDecrementPtr = ptr;
	}
	else if(command == strCommandStepperDecelerationBuffer)
	{
		_type = ReplyType::StepperDecelerationBuffer;

		std::shared_ptr<ReplyStepperDecelerationBuffer> ptr (new ReplyStepperDecelerationBuffer);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperDecelerationBufferPtr = ptr;
	}
	else if(command == strCommandStepperDecelerationBufferIncrement)
	{
		_type = ReplyType::StepperDecelerationBufferIncrement;

		std::shared_ptr<ReplyStepperDecelerationBufferIncrement> ptr (new ReplyStepperDecelerationBufferIncrement);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperDecelerationBufferIncrementPtr = ptr;
	}
	else if(command == strCommandStepperEnable)
	{
		_type = ReplyType::StepperEnable;

		std::shared_ptr<ReplyStepperEnable> ptr (new ReplyStepperEnable);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
			ptr->enabled = ds["enabled"];
		}

		_stepperEnablePtr = ptr;
	}
	else if(command == strCommandStepperForward)
	{
		_type = ReplyType::StepperForward;

		std::shared_ptr<ReplyStepperForward> ptr (new ReplyStepperForward);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
			ptr->forward = ds["forward"];
		}

		_stepperForwardPtr = ptr;
	}
	else if(command == strCommandStepperSteps)
	{
		_type = ReplyType::StepperSteps;

		std::shared_ptr<ReplyStepperSteps> ptr (new ReplyStepperSteps);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperStepsPtr = ptr;
	}
	else if(command == strCommandStepperRun)
	{
		_type = ReplyType::StepperRun;

		std::shared_ptr<ReplyStepperRun> ptr (new ReplyStepperRun);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperRunPtr = ptr;
	}
	else if(command == strCommandStepperConfigHome)
	{
		_type = ReplyType::StepperConfigHome;

		std::shared_ptr<ReplyStepperConfigHome> ptr (new ReplyStepperConfigHome);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperConfigHomePtr = ptr;
	}
	else if(command == strCommandStepperMove)
	{
		//not implemented
		_type = ReplyType::StepperMove;
		pLogger->LogError("ReplyTranslator::parseReply unimplemented reply translator: " + strCommandStepperMove);
	}
	else if(command == strCommandStepperQuery)
	{
		_type = ReplyType::StepperQuery;

		std::shared_ptr<ReplyStepperQuery> ptr (new ReplyStepperQuery);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty())
		{
			ptr->index = ds["index"];
			ptr->state = ds["state"].toString();
			ptr->bEnabled = ds["enabled"];
			ptr->bForward = ds["forward"];
			ptr->locatorIndex = ds["locatorIndex"];
			ptr->locatorLineNumberStart = ds["locatorLineNumberStart"];
			ptr->locatorLineNumberTerminal = ds["locatorLineNumberTerminal"];
			ptr->homeOffset = ds["homeOffset"];
			ptr->lowClks = ds["lowClks"];
			ptr->highClks = ds["highClks"];
			ptr->accelerationBuffer = ds["accelerationBuffer"];
			ptr->accelerationBufferDecrement = ds["accelerationBufferDecrement"];
			ptr->decelerationBuffer = ds["decelerationBuffer"];
			ptr->decelerationBufferIncrement = ds["decelerationBufferIncrement"];
		}

		_stepperQueryPtr = ptr;
	}
	else if(command == strCommandStepperSetState)
	{
		_type = ReplyType::StepperSetState;

		std::shared_ptr<ReplyStepperSetState> ptr (new ReplyStepperSetState);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
		}

		_stepperSetStatePtr = ptr;
	}
	else if(command == strCommandLocatorQuery)
	{
		_type = ReplyType::LocatorQuery;

		std::shared_ptr<ReplyLocatorQuery> ptr (new ReplyLocatorQuery);
		//common attributes
		ptr->originalString = _reply;
		ptr->commandKey = commandKey;
		ptr->commandId = commandId;
		ptr->errorInfo = errorInfo;
		//specific attributes
		if(errorInfo.empty()) {
			ptr->index = ds["index"];
			ptr->lowInput = ds["lowInput"];
		}

		_locatorQueryPtr = ptr;
	}
	else
	{
		throw Poco::Exception("ReplyTranslator::parseReply unknown reply");
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

std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> ReplyTranslator::ToDeviceQueryFuse()
{
	return _deviceQueryFusePtr;
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

std::shared_ptr<ReplyTranslator::ReplySteppersPowerOn> ReplyTranslator::ToSteppersPowerOn()
{
	return _steppersPowerOnPtr;
}

std::shared_ptr<ReplyTranslator::ReplySteppersPowerOff> ReplyTranslator::ToSteppersPowerOff()
{
	return _steppersPowerOffPtr;
}

std::shared_ptr<ReplyTranslator::ReplySteppersQueryPower> ReplyTranslator::ToSteppersQueryPower()
{
	return _steppersQueryPowerPtr;
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

std::shared_ptr<ReplyTranslator::ReplyStepperForward> ReplyTranslator::ToStepperForward()
{
	return _stepperForwardPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperSteps> ReplyTranslator::ToStepperSteps()
{
	return _stepperStepsPtr;
}

std::shared_ptr<ReplyTranslator::ReplyStepperRun> ReplyTranslator::ToStepperRun()
{
	return _stepperRunPtr;
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

std::shared_ptr<ReplyTranslator::ReplyStepperSetState> ReplyTranslator::ToStepperSetState()
{
	return _stepperSetStatePtr;
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
