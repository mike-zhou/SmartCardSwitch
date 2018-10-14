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

std::string CommandDeviceConnect::GetInitialState()
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
std::string CommandBdcsPowerOn::GetInitialState()
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
std::string CommandBdcsPowerOff::GetInitialState()
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
std::string CommandBdcsQueryPower::GetInitialState()
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
CommandBdcOperation::CommandBdcOperation(unsigned int bdcIndex, BdcMode initialMode, BdcMode finalMode, unsigned long delayMs)
{
	_bdcIndex = bdcIndex;
	_initialMode = initialMode;
	_finalMode = finalMode;
	_delayMs = delayMs;
}

std::string CommandBdcOperation::GetInitialState()
{
	std::string mode;

	switch(_initialMode)
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
		pLogger->LogError("CommandBdcOperation::GetInitialState wrong mode: " + std::to_string(_initialMode));
	}

	return mode;
}

std::string CommandBdcOperation::GetFinalState()
{
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

	return mode;
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

	switch(_initialMode)
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
		pLogger->LogError("CommandBdcOperation::ToCommandUndo wrong mode: " + std::to_string(_initialMode));
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
