/*
 * Command.cpp
 *
 *  Created on: Oct 14, 2018
 *      Author: user1
 */

#include "Command.h"
#include "Logger.h"

extern Logger * pLogger;

unsigned long Command::_commandIdSeed = 1;

///////////////////////////////////////////////////////////
// CommandDevicesGet
///////////////////////////////////////////////////////////
std::string CommandDevicesGet::ToCommand()
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

std::string CommandDeviceConnect::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device connect\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"device\":\"" + _deviceName +"\"";
	cmd += "}";

	return cmd;
}

std::string CommandDeviceConnect::ToCommandUndo()
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
std::string CommandDeviceQueryPower::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcsPowerOn
///////////////////////////////////////////////////////
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

std::string CommandBdcsPowerOn::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

std::string CommandBdcsPowerOn::ToCommandUndo()
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

std::string CommandBdcsPowerOff::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

std::string CommandBdcsPowerOff::ToCommandUndo()
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
std::string CommandBdcsQueryPower::GetUndoState()
{
	std::string empty;
	return empty;
}

std::string CommandBdcsQueryPower::GetFinalState()
{
	std::string empty;
	return empty;
}

std::string CommandBdcsQueryPower::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

std::string CommandBdcsQueryPower::ToCommandUndo()
{
	std::string empty;
	return empty;
}

///////////////////////////////////////////////////////
// CommandBdcOperation
///////////////////////////////////////////////////////
CommandBdcOperation::CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode, unsigned long delayMs)
{
	_bdcIndex = bdcIndex;
	_undoMode = undoMode;
	_finalMode = finalMode;
	_delayMs = delayMs;
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

	state = "\"bdcIndex\":" + std::to_string(_bdcIndex) + ",\"mode\":\"" + mode + "\"";
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

	state = "\"bdcIndex\":" + std::to_string(_bdcIndex) + ",\"mode\":\"" + mode + "\"";
	return state;
}

std::string CommandBdcOperation::ToCommand()
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
		cmd = cmd + "\"delayMs\":" + std::to_string(_delayMs);
		cmd += "}";
	}

	return cmd;
}

std::string CommandBdcOperation::ToCommandUndo()
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
		cmd = cmd + "\"delayMs\":" + std::to_string(_delayMs);
		cmd += "}";
	}

	return cmd;
}


///////////////////////////////////////////////////////////
// CommandStepperQueryClkPeriod
///////////////////////////////////////////////////////////
std::string CommandStepperQueryClkPeriod::ToCommand()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper query clkPeriod\",";
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

std::string CommandStepperConfigStep::ToCommand()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"lowClks\":" + std::to_string(_lowClks)+ ",\"highClks\":" + std::to_string(_highClks);
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

std::string CommandStepperAccelerationBuffer::ToCommand()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"accelerationBuffer\":" + std::to_string(_value);
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

std::string CommandStepperAccelerationBufferDecrement::ToCommand()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"accelerationBufferDecrement\":" + std::to_string(_value);
	return state;
}


///////////////////////////////////////////////////////////
// CommandStepperDecelrationBuffer
///////////////////////////////////////////////////////////
CommandStepperDecelrationBuffer::CommandStepperDecelrationBuffer(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperDecelrationBuffer::ToCommand()
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

std::string CommandStepperDecelrationBuffer::GetFinalState()
{
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"decelerationBuffer\":" + std::to_string(_value);
	return state;
}

///////////////////////////////////////////////////////////
// CommandStepperDecelrationBufferIncrement
///////////////////////////////////////////////////////////
CommandStepperDecelrationBufferIncrement::CommandStepperDecelrationBufferIncrement(unsigned int stepperIndex, unsigned long value)
{
	_stepperIndex = stepperIndex;
	_value = value;
}

std::string CommandStepperDecelrationBufferIncrement::ToCommand()
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

std::string CommandStepperDecelrationBufferIncrement::GetFinalState()
{
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"decelerationBufferIncrement\":" + std::to_string(_value);
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

std::string CommandStepperEnable::ToCommand()
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

std::string CommandStepperEnable::ToCommandUndo()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"enabled\":";
	if(_enable) {
		state = state + "false";
	}
	else {
		state = state + "true";
	}

	return state;
}

std::string CommandStepperEnable::GetFinalState()
{
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"enabled\":";
	if(_enable) {
		state = state + "true";
	}
	else {
		state = state + "false";
	}

	return state;
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

std::string CommandStepperConfigHome::ToCommand()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":0";
	return state;
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

std::string CommandStepperMove::ToCommand()
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

std::string CommandStepperMove::ToCommandUndo()
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
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":" + std::to_string(_position);
	return state;
}

std::string CommandStepperMove::GetFinalState()
{
	std::string state = "\"stepperIndex\":" + std::to_string(_stepperIndex) + ",\"position\":";
	if(_forward) {
		state = state + std::to_string(_position + _steps);
	}
	else {
		state = state + std::to_string(_position - _steps);
	}
	return state;
}


