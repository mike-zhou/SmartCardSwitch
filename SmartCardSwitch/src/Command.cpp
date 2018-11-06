/*
 * Command.cpp
 *
 *  Created on: Oct 14, 2018
 *      Author: user1
 */

#include "Command.h"
#include "Logger.h"

extern Logger * pLogger;

unsigned long DeviceCommand::_commandIdSeed = 1;

///////////////////////////////////////////////////////////
// CommandDevicesGet
///////////////////////////////////////////////////////////
std::string CommandDevicesGet::CommandKey()
{
	return std::string("devices get");
}

std::string CommandDevicesGet::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"devices get\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////////////////////
// CommandDeviceConnect
//////////////////////////////////////////////////////////
CommandDeviceConnect::CommandDeviceConnect(const std::string& deviceName)
{
	_deviceName = deviceName;
}

std::string CommandDeviceConnect::CommandKey()
{
	return std::string("device connect");
}

std::string CommandDeviceConnect::GetUndoState()
{
	std::string state = "disconnected from " + _deviceName;
	return state;
}

std::string CommandDeviceConnect::GetFinalState()
{
	std::string state;
	state = "connected to " + _deviceName;
	return state;
}

std::string CommandDeviceConnect::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device connect\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"device\":\"" + _deviceName +"\"";
	cmd += "}";

	return cmd;
}

std::string CommandDeviceConnect::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device disconnect\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId()) + ",";
	cmd = cmd + "\"device\":\"" + _deviceName +"\"";
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////////
// CommandDeviceQueryPower
///////////////////////////////////////////////////////////
std::string CommandDeviceQueryPower::CommandKey()
{
	return std::string("device query power");
}

std::string CommandDeviceQueryPower::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////////
// CommandDeviceQueryFuse
///////////////////////////////////////////////////////////
std::string CommandDeviceQueryFuse::CommandKey()
{
	return std::string("device query fuse");
}

std::string CommandDeviceQueryFuse::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device query fuse\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcsPowerOn
///////////////////////////////////////////////////////
std::string CommandBdcsPowerOn::CommandKey()
{
	return std::string("bdcs power on");
}

std::string CommandBdcsPowerOn::GetUndoState()
{
	std::string state = "bdcs is powered off";
	return state;
}

std::string CommandBdcsPowerOn::GetFinalState()
{
	std::string state = "bdcs is powered on";
	return state;
}

std::string CommandBdcsPowerOn::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

std::string CommandBdcsPowerOn::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcsPowerOff
///////////////////////////////////////////////////////
std::string CommandBdcsPowerOff::CommandKey()
{
	return std::string("bdc power off");
}

std::string CommandBdcsPowerOff::GetUndoState()
{
	std::string state = "bdcs is powered on";
	return state;
}

std::string CommandBdcsPowerOff::GetFinalState()
{
	std::string state = "bdcs is powered off";
	return state;
}

std::string CommandBdcsPowerOff::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

std::string CommandBdcsPowerOff::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcsPowerQuery
///////////////////////////////////////////////////////
std::string CommandBdcsQueryPower::CommandKey()
{
	return std::string("bdcs query power");
}

std::string CommandBdcsQueryPower::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcOperation
///////////////////////////////////////////////////////
CommandBdcOperation::CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode)
{
	_bdcIndex = bdcIndex;
	_undoMode = undoMode;
	_finalMode = finalMode;
}

std::string CommandBdcOperation::CommandKey()
{
	std::string key;

	switch(_finalMode)
	{
	case BdcMode::COAST:
		key = "bdc coast";
		break;
	case BdcMode::REVERSE:
		key = "bdc reverse";
		break;
	case BdcMode::FORWARD:
		key = "bdc forward";
		break;
	case BdcMode::BREAK:
		key = "bdc break";
		break;
	default:
		pLogger->LogError("CommandBdcOperation::CommandKey unknown bdc mode: " + std::to_string(_finalMode));
		break;
	}

	return key;
}

std::string CommandBdcOperation::GetUndoState()
{
	std::string state;
	std::string mode;

	switch(_undoMode)
	{
	case BdcMode::COAST:
		mode = "coast";
		break;

	case BdcMode::REVERSE:
		mode = "reverse";
		break;

	case BdcMode::FORWARD:
		mode = "forward";
		break;

	case BdcMode::BREAK:
		mode = "break";
		break;

	default:
		pLogger->LogError("CommandBdcOperation::GetInitialState wrong mode: " + std::to_string(_undoMode));
	}

	state = "{\"bdcIndex\":" + std::to_string(_bdcIndex) + ",\"mode\":\"" + mode + "\"}";
	return state;
}

std::string CommandBdcOperation::GetFinalState()
{
	std::string state;
	std::string mode;

	switch(_finalMode)
	{
	case BdcMode::COAST:
		mode = "coast";
		break;

	case BdcMode::REVERSE:
		mode = "reverse";
		break;

	case BdcMode::FORWARD:
		mode = "forward";
		break;

	case BdcMode::BREAK:
		mode = "break";
		break;

	default:
		pLogger->LogError("CommandBdcOperation::GetFinalState wrong mode: " + std::to_string(_finalMode));
	}

	state = "{\"bdcIndex\":" + std::to_string(_bdcIndex) + ",\"mode\":\"" + mode + "\"}";
	return state;
}

std::string CommandBdcOperation::ToJsonCommandString()
{
	std::string cmd;
	std::string mode;

	switch(_finalMode)
	{
	case BdcMode::COAST:
		mode = "coast";
		break;

	case BdcMode::REVERSE:
		mode = "reverse";
		break;

	case BdcMode::FORWARD:
		mode = "forward";
		break;

	case BdcMode::BREAK:
		mode = "break";
		break;

	default:
		pLogger->LogError("CommandBdcOperation::ToCommand wrong mode: " + std::to_string(_finalMode));
	}

	//cmd is empty if mode is wrong.
	if(mode.size() > 0) {
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc " + mode + "\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd += "}";
	}

	return cmd;
}

std::string CommandBdcOperation::ToJsonCommandUndoString()
{
	std::string cmd;
	std::string mode;

	switch(_undoMode)
	{
	case BdcMode::COAST:
		mode = "coast";
		break;

	case BdcMode::REVERSE:
		mode = "reverse";
		break;

	case BdcMode::FORWARD:
		mode = "forward";
		break;

	case BdcMode::BREAK:
		mode = "break";
		break;

	default:
		pLogger->LogError("CommandBdcOperation::ToCommandUndo wrong mode: " + std::to_string(_undoMode));
	}

	//cmd is empty if mode is wrong.
	if(mode.size() > 0) {
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc " + mode + "\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd += "}";
	}

	return cmd;
}

//////////////////////////////////////////
// CommandBdcQuery
//////////////////////////////////////////
CommandBdcQuery::CommandBdcQuery(unsigned int bdcIndex)
{
	_bdcIndex = bdcIndex;
}

std::string CommandBdcQuery::CommandKey()
{
	return std::string("bdc query");
}

std::string CommandBdcQuery::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdc query\",";
	cmd = cmd + "\"index\":" + std::to_string(_bdcIndex) + ",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////////
// CommandStepperQueryResolution
///////////////////////////////////////////////////////////
std::string CommandStepperQueryResolution::CommandKey()
{
	return std::string("stepper query resolution");
}

std::string CommandStepperQueryResolution::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper query resolution\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////////////////////
//CommandSteppersPowerOn
//////////////////////////////////////////////////////////
std::string CommandSteppersPowerOn::CommandKey()
{
	return std::string("steppers power on");
}

std::string CommandSteppersPowerOn::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"steppers power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////////////////////
//CommandSteppersPowerOff
//////////////////////////////////////////////////////////
std::string CommandSteppersPowerOff::CommandKey()
{
	return std::string("steppers power off");
}

std::string CommandSteppersPowerOff::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"steppers power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////////////////////
//CommandSteppersQueryPower
//////////////////////////////////////////////////////////
std::string CommandSteppersQueryPower::CommandKey()
{
	return std::string("steppers query power");
}

std::string CommandSteppersQueryPower::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"steppers query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////////
// CommandStepperConfigStep
///////////////////////////////////////////////////////////
CommandStepperConfigStep::CommandStepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks)
{
	_stepperIndex = stepperIndex;
	_lowClks = lowClks;
	_highClks = highClks;
}

std::string CommandStepperConfigStep::CommandKey()
{
	return std::string("stepper config step");
}

std::string CommandStepperConfigStep::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper config step\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"lowClks\":" + std::to_string(_lowClks) + ",";
	cmd = cmd + "\"highClks\":" + std::to_string(_highClks);
	cmd += "}";

	return cmd;
}

std::string CommandStepperConfigStep::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"lowClks\":" + std::to_string(_lowClks)+ ",\"highClks\":" + std::to_string(_highClks) + "}";
	return state;
}


///////////////////////////////////////////////////////////
// CommandStepperAccelerationBuffer
///////////////////////////////////////////////////////////
CommandStepperAccelerationBuffer::CommandStepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperAccelerationBuffer::CommandKey()
{
	return std::string("stepper acceleration buffer");
}

std::string CommandStepperAccelerationBuffer::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper acceleration buffer\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"value\":" + std::to_string(_value);
	cmd += "}";

	return cmd;
}

std::string CommandStepperAccelerationBuffer::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"accelerationBuffer\":" + std::to_string(_value) + "}";
	return state;
}


///////////////////////////////////////////////////////////
// CommandStepperAccelerationBufferDecrement
///////////////////////////////////////////////////////////
CommandStepperAccelerationBufferDecrement::CommandStepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperAccelerationBufferDecrement::CommandKey()
{
	return std::string("stepper acceleration buffer decrement");
}

std::string CommandStepperAccelerationBufferDecrement::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper acceleration buffer decrement\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"value\":" + std::to_string(_value);
	cmd += "}";

	return cmd;
}

std::string CommandStepperAccelerationBufferDecrement::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"accelerationBufferDecrement\":" + std::to_string(_value) + "}";
	return state;
}


///////////////////////////////////////////////////////////
// CommandStepperDecelrationBuffer
///////////////////////////////////////////////////////////
CommandStepperDecelerationBuffer::CommandStepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperDecelerationBuffer::CommandKey()
{
	return std::string("stepper deceleration buffer");
}

std::string CommandStepperDecelerationBuffer::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper deceleration buffer\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"value\":" + std::to_string(_value);
	cmd += "}";

	return cmd;
}

std::string CommandStepperDecelerationBuffer::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"decelerationBuffer\":" + std::to_string(_value) + "}";
	return state;
}

///////////////////////////////////////////////////////////
// CommandStepperDecelrationBufferIncrement
///////////////////////////////////////////////////////////
CommandStepperDecelerationBufferIncrement::CommandStepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperDecelerationBufferIncrement::CommandKey()
{
	return std::string("stepper deceleration buffer increment");
}

std::string CommandStepperDecelerationBufferIncrement::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper deceleration buffer increment\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"value\":" + std::to_string(_value);
	cmd += "}";

	return cmd;
}

std::string CommandStepperDecelerationBufferIncrement::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"decelerationBufferIncrement\":" + std::to_string(_value) + "}";
	return state;
}

///////////////////////////////////////////////////////////
// CommandStepperEnable
///////////////////////////////////////////////////////////
CommandStepperEnable::CommandStepperEnable(unsigned int stepperIndex, bool enable)
{
	_stepperIndex = stepperIndex;
	_enable = enable;
}

std::string CommandStepperEnable::CommandKey()
{
	return std::string("stepper enable");
}

std::string CommandStepperEnable::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper enable\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"enable\":" + std::string(_enable?"true":"false");
	cmd += "}";

	return cmd;
}

std::string CommandStepperEnable::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper enable\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"enable\":" + std::string(_enable?"false":"true");
	cmd += "}";

	return cmd;
}

std::string CommandStepperEnable::GetUndoState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"enabled\":";
	if(_enable) {
		state = state + "false";
	}
	else {
		state = state + "true";
	}
	state = state + "}";

	return state;
}

std::string CommandStepperEnable::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"enabled\":";
	if(_enable) {
		state = state + "true";
	}
	else {
		state = state + "false";
	}
	state = state + "}";

	return state;
}


///////////////////////////////////////////////
// CommandStepperForward
///////////////////////////////////////////////
CommandStepperForward::CommandStepperForward(unsigned int stepperIndex, bool bForward)
{
	_stepperIndex = stepperIndex;
	_bForward = bForward;
}

std::string CommandStepperForward::CommandKey()
{
	return std::string("stepper forward");
}

std::string CommandStepperForward::GetUndoState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"forward\":";
	if(_bForward) {
		state = state + "false";
	}
	else {
		state = state + "true";
	}
	state = state + "}";

	return state;
}

std::string CommandStepperForward::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"forward\":";
	if(_bForward) {
		state = state + "true";
	}
	else {
		state = state + "false";
	}
	state = state + "}";

	return state;
}

std::string CommandStepperForward::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper forward\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"forward\":" + std::string(_bForward?"true":"false");
	cmd += "}";

	return cmd;
}

std::string CommandStepperForward::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper forward\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"forward\":" + std::string(_bForward?"false":"true");
	cmd += "}";

	return cmd;
}

////////////////////////////////////////
// CommandStepperSteps
////////////////////////////////////////
CommandStepperSteps::CommandStepperSteps(unsigned int stepperIndex, unsigned long steps)
{
	_stepperIndex = stepperIndex;
	_steps = steps;
}

std::string CommandStepperSteps::CommandKey()
{
	return std::string("stepper steps");
}

std::string CommandStepperSteps::GetUndoState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"steps\":" + std::to_string(_steps) + "}";
	return state;
}

std::string CommandStepperSteps::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"steps\":" + std::to_string(_steps) + "}";
	return state;
}

std::string CommandStepperSteps::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper steps\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"steps\":" + std::to_string(_steps);
	cmd += "}";

	return cmd;
}

std::string CommandStepperSteps::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper steps\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"steps\":" + std::to_string(_steps);
	cmd += "}";

	return cmd;
}


///////////////////////////////////////////
// CommandStepperRun
///////////////////////////////////////////
CommandStepperRun::CommandStepperRun(unsigned int stepperIndex, unsigned long initialPosition, unsigned long finalPosition)
{
	_stepperIndex = stepperIndex;
	_initialPosition = initialPosition;
	_finalPosition = finalPosition;
}

std::string CommandStepperRun::CommandKey()
{
	return std::string("stepper run");
}

std::string CommandStepperRun::GetUndoState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":" + std::to_string(_initialPosition) + "}";
	return state;
}

std::string CommandStepperRun::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":" + std::to_string(_finalPosition) + "}";
	return state;
}

std::string CommandStepperRun::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper run\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex);
	cmd += "}";

	return cmd;
}

std::string CommandStepperRun::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper run\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex);
	cmd += "}";

	return cmd;
}


///////////////////////////////////////////////////////////
// CommandStepperConfigHome
///////////////////////////////////////////////////////////
CommandStepperConfigHome::CommandStepperConfigHome(unsigned int stepperIndex,
													unsigned int locatorIndex,
													unsigned int lineNumberStart,
													unsigned int lineNumberTerminal)
{
	_stepperIndex = stepperIndex;
	_locatorIndex = locatorIndex;
	_lineNumberStart = lineNumberStart;
	_lineNumberTerminal = lineNumberTerminal;
}

std::string CommandStepperConfigHome::CommandKey()
{
	return std::string("stepper config home");
}

std::string CommandStepperConfigHome::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper config home\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"locatorIndex\":" + std::to_string(_locatorIndex) + ",";
	cmd = cmd + "\"lineNumberStart\":" + std::to_string(_lineNumberStart) + ",";
	cmd = cmd + "\"lineNumberTerminal\":" + std::to_string(_lineNumberTerminal);
	cmd += "}";

	return cmd;
}

std::string CommandStepperConfigHome::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":0}";
	return state;
}

///////////////////////////////////////////////////////////
//CommandStepperQuery
///////////////////////////////////////////////////////////
CommandStepperQuery::CommandStepperQuery(unsigned int stepperIndex)
{
	_stepperIndex = stepperIndex;
}

std::string CommandStepperQuery::CommandKey()
{
	return std::string("stepper query");
}

std::string CommandStepperQuery::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper query\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex);
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////////
// CommandStepperMove
///////////////////////////////////////////////////////////
CommandStepperMove::CommandStepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps)
{
	_stepperIndex = stepperIndex;
	_position = position;
	_forward = forward;
	_steps = steps;
}

std::string CommandStepperMove::CommandKey()
{
	return std::string("stepper move");
}

std::string CommandStepperMove::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper move\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"forward\":" + std::string(_forward?"true":"false") + ",";
	cmd = cmd + "\"steps\":" + std::to_string(_steps);
	cmd += "}";

	return cmd;
}

std::string CommandStepperMove::ToJsonCommandUndoString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper move\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandUndoId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"forward\":" + std::string(_forward?"false":"true") + ",";
	cmd = cmd + "\"steps\":" + std::to_string(_steps);
	cmd += "}";

	return cmd;
}

std::string CommandStepperMove::GetUndoState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":" + std::to_string(_position) + "}";
	return state;
}

std::string CommandStepperMove::GetFinalState()
{
	std::string state = "{\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":";
	if(_forward) {
		state = state + std::to_string(_position + _steps);
	}
	else {
		state = state + std::to_string(_position - _steps);
	}
	state = state + "}";

	return state;
}

/////////////////////////////////////////////
// CommandLoctorQuery
/////////////////////////////////////////////
CommandLocatorQuery::CommandLocatorQuery(unsigned int locatorIndex)
{
	_locatorIndex = locatorIndex;
}

std::string CommandLocatorQuery::CommandKey()
{
	return std::string("locator query");
}

std::string CommandLocatorQuery::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"locator query\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_locatorIndex);
	cmd += "}";

	return cmd;
}

