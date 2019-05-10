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

///////////////////////////////////////////////////////////
// CommandDeviceDelay
///////////////////////////////////////////////////////////
CommandDeviceDelay::CommandDeviceDelay(unsigned int clks)
{
	_clks = clks;
}

std::string CommandDeviceDelay::CommandKey()
{
	return std::string("device delay");
}

std::string CommandDeviceDelay::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"device delay\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd = cmd + ",\"clks\":" + std::to_string(_clks);
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

std::string CommandBdcsPowerOn::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"bdcs power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

///////////////////////////////////////////////////////
// CommandBdcsPowerOff
///////////////////////////////////////////////////////
std::string CommandBdcsPowerOff::CommandKey()
{
	return std::string("bdcs power off");
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
CommandBdcOperation::CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode, unsigned int lowClks, unsigned int highClks, unsigned int cycles)
{
	_bdcIndex = bdcIndex;
	_undoMode = undoMode;
	_finalMode = finalMode;
	_lowClks = lowClks;
	_highClks = highClks;
	_cycles = cycles;
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

std::string CommandBdcOperation::ToJsonCommandString()
{
	std::string cmd;

	switch(_finalMode)
	{
	case BdcMode::COAST:
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc coast\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd += "}";
		break;

	case BdcMode::REVERSE:
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc reverse\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd = cmd + ",\"lowClks\":" + std::to_string(_lowClks);
		cmd = cmd + ",\"highClks\":" + std::to_string(_highClks);
		cmd = cmd + ",\"cycles\":" + std::to_string(_cycles);
		cmd += "}";
		break;

	case BdcMode::FORWARD:
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc forward\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd = cmd + ",\"lowClks\":" + std::to_string(_lowClks);
		cmd = cmd + ",\"highClks\":" + std::to_string(_highClks);
		cmd = cmd + ",\"cycles\":" + std::to_string(_cycles);
		cmd += "}";
		break;

	case BdcMode::BREAK:
		cmd = "{";
		cmd = cmd + "\"command\":\"bdc break\",";
		cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
		cmd = cmd + ",\"index\":" + std::to_string(_bdcIndex);
		cmd += "}";
		break;

	default:
		pLogger->LogError("CommandBdcOperation::ToCommand wrong mode: " + std::to_string(_finalMode));
	}

	//cmd is empty if mode is wrong.
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

std::string CommandStepperSteps::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper steps\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"value\":" + std::to_string(_steps);
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

///////////////////////////////////////////////
// CommandStepperForceState
////////////////////////////////////////////////
CommandStepperSetState::CommandStepperSetState(unsigned int stepperIndex, unsigned int state)
{
	_stepperIndex = stepperIndex;
	_state = state;
}

std::string CommandStepperSetState::CommandKey()
{
	return std::string("stepper set state");
}

std::string CommandStepperSetState::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper set state\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"state\":" + std::to_string(_state);
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

/////////////////////////////////////////////////
// CommandStepperForwardClockwise
/////////////////////////////////////////////////
CommandStepperForwardClockwise::CommandStepperForwardClockwise(unsigned int stepperIndex, bool forwardClockwise)
{
	_stepperIndex = stepperIndex;
	_forwardClockwise = forwardClockwise;
}

std::string CommandStepperForwardClockwise::CommandKey()
{
	return std::string("stepper forward clockwise");
}

std::string CommandStepperForwardClockwise::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"stepper forward clockwise\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_stepperIndex) + ",";
	cmd = cmd + "\"forwardClockwise\":" + std::string(_forwardClockwise?"1":"0");
	cmd += "}";

	return cmd;
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


//////////////////////////////////////////
// CommandOptPowerOn
//////////////////////////////////////////
CommandOptPowerOn::CommandOptPowerOn()
{

}

std::string CommandOptPowerOn::CommandKey()
{
	return std::string("opt power on");
}

std::string CommandOptPowerOn::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"opt power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////
// CommandOptPowerOff
//////////////////////////////////////////
CommandOptPowerOff::CommandOptPowerOff()
{

}

std::string CommandOptPowerOff::CommandKey()
{
	return std::string("opt power off");
}

std::string CommandOptPowerOff::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"opt power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////
// CommandOptQueryPower
//////////////////////////////////////////
CommandOptQueryPower::CommandOptQueryPower()
{

}

std::string CommandOptQueryPower::CommandKey()
{
	return std::string("opt query power");
}

std::string CommandOptQueryPower::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"opt query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId());
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////
// CommandDcmPowerOn
//////////////////////////////////////////
CommandDcmPowerOn::CommandDcmPowerOn(unsigned int index)
{
	_index = index;
}

std::string CommandDcmPowerOn::CommandKey()
{
	return std::string("dcm power on");
}

std::string CommandDcmPowerOn::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"dcm power on\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_index);
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////
// CommandDcmPowerOff
//////////////////////////////////////////
CommandDcmPowerOff::CommandDcmPowerOff(unsigned int index)
{
	_index = index;
}

std::string CommandDcmPowerOff::CommandKey()
{
	return std::string("dcm power off");
}

std::string CommandDcmPowerOff::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"dcm power off\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_index);
	cmd += "}";

	return cmd;
}

//////////////////////////////////////////
// CommandDcmQueryPower
//////////////////////////////////////////
CommandDcmQueryPower::CommandDcmQueryPower(unsigned int index)
{
	_index = index;
}

std::string CommandDcmQueryPower::CommandKey()
{
	return std::string("dcm query power");
}

std::string CommandDcmQueryPower::ToJsonCommandString()
{
	std::string cmd;

	cmd = "{";
	cmd = cmd + "\"command\":\"dcm query power\",";
	cmd = cmd + "\"commandId\":" + std::to_string(CommandId()) + ",";
	cmd = cmd + "\"index\":" + std::to_string(_index);
	cmd += "}";

	return cmd;
}

