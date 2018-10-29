/*
 * CommandParser.h
 *
 *  Created on: Oct 8, 2018
 *      Author: user1
 */

#ifndef COMMANDPARSER_H_
#define COMMANDPARSER_H_

#include <memory>
#include <string>
#include "Poco/Format.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

enum CommandType
{
	Invalid = -1,
	DevicesGet,
	DeviceConnect,
	DeviceQueryPower,
	DeviceQueryFuse,
	OptPowerOn,
	OptPowerOff,
	OptQueryPower,
	DcmPowerOn,
	DcmPowerOff,
	DcmQueryPower,
	BdcsPowerOn,
	BdcsPowerOff,
	BdcsQueryPower,
	BdcCoast,
	BdcReverse,
	BdcForward,
	BdcBreak,
	BdcQuery,
	SteppersPowerOn,
	SteppersPowerOff,
	SteppersQueryPower,
	StepperQueryResolution,
	StepperConfigStep,
	StepperAccelerationBuffer,
	StepperAccelerationBufferDecrement,
	StepperDecelerationBuffer,
	StepperDecelerationBufferIncrement,
	StepperEnable,
	StepperForward,
	StepperSteps,
	StepperRun,
	StepperConfigHome,
	StepperQuery,
	LocatorQuery
};

//{
//	"command":"devices get"
//}
class CommandDevicesGet
{
public:
	CommandDevicesGet(unsigned long commandId) { _commandId = commandId; }

	CommandType Type() { return CommandType::DevicesGet; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string empty;
		return empty;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"device connect",
//	"device": "device12345"
//}
class CommandDeviceConnect
{
public:
	CommandDeviceConnect(unsigned long commandId, std::string deviceName)
	{
		_commandId = commandId;
		_deviceName = deviceName;
	}

	CommandType Type() { return CommandType::DeviceConnect; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string empty;
		return empty;
	}

	std::string DeviceName() { return _deviceName; }

private:
	std::string _deviceName;
	unsigned long _commandId;
};


//{
//	"command":"device query power",
//	"commandId":1
//}
class CommandDeviceQueryPower
{
public:
	CommandDeviceQueryPower(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::DeviceQueryPower; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string cmd = "C 2 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"device query fuse",
//	"commandId":1
//}
class CommandDeviceQueryFuse
{
public:
	CommandDeviceQueryFuse(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::DeviceQueryFuse; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string cmd = "C 3 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"opt power on"
//}

//{
//	"command":"opt power off"
//}

//{
//	"command":"opt query power"
//}

//{
//	"command":"dcm power on",
//	"index":0
//}

//{
//	"command":"dcm power off",
//	"index":0
//}

//{
//	"command":"dcm query power",
//	"index":0
//}

//{
//	"command":"bdcs power on"
//}
class CommandBdcsPowerOn
{
public:
	CommandBdcsPowerOn(unsigned long commandId) { _commandId = commandId; }
	CommandType Type() { return CommandType::BdcsPowerOn; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string cmd = "C 40 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"bdcs power off"
//}
class CommandBdcsPowerOff
{
public:
	CommandBdcsPowerOff(unsigned long commandId) { _commandId = commandId; }
	CommandType Type() { return CommandType::BdcsPowerOff; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string cmd = "C 41" + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"bdcs query power"
//}
class CommandBdcsQueryPower
{
public:
	CommandBdcsQueryPower(unsigned long commandId) { _commandId = commandId; }
	CommandType Type() { return CommandType::BdcsQueryPower; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string cmd = "C 42" + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"bdc coast",
//	"index":0
//}
class CommandBdcCoast
{
public:
	CommandBdcCoast(unsigned int bdcIndex, unsigned long commandId)
	{
		_bdcIndex = bdcIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::BdcCoast; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 43 %d %d"), _bdcIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
	unsigned int _bdcIndex;
};

//{
//	"command":"bdc reverse",
//	"index":0
//}
class CommandBdcReverse
{
public:
	CommandBdcReverse(unsigned int bdcIndex, unsigned long commandId)
	{
		_bdcIndex = bdcIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::BdcReverse; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 44 %d %d"), _bdcIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	unsigned int _bdcIndex;
	unsigned long _commandId;
};

//{
//	"command":"bdc forward",
//	"index":0
//}
class CommandBdcForward
{
public:
	CommandBdcForward(unsigned int bdcIndex, unsigned long commandId)
	{
		_bdcIndex = bdcIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::BdcForward; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 45 %d %d"), _bdcIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	int _bdcIndex;
	unsigned long _commandId;
};

//{
//	"command":"bdc break",
//	"index":0
//}
class CommandBdcBreak
{
public:
	CommandBdcBreak(unsigned int bdcIndex, unsigned long commandId)
	{
		_bdcIndex = bdcIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::BdcBreak; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 46 %d %d"), _bdcIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	unsigned int _bdcIndex;
	unsigned long _commandId;
};

//{
//	"command":"bdc query",
//	"index":0
//}
class CommandBdcQuery
{
public:
	CommandBdcQuery(unsigned int bdcIndex, unsigned long commandId)
	{
		_bdcIndex = bdcIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::BdcQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 47 %d %d"), _bdcIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	unsigned int _bdcIndex;
	unsigned long _commandId;
};

//{
//	"command":"steppers power on"
//}
class CommandSteppersPowerOn
{
public:
	CommandSteppersPowerOn(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::SteppersPowerOn; }

	std::string ToString()
	{
		std::string cmd = "C 13 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"steppers power off"
//}
class CommandSteppersPowerOff
{
public:
	CommandSteppersPowerOff(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::SteppersPowerOff; }

	std::string ToString()
	{
		std::string cmd = "C 14 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"steppers query power"
//}
class CommandSteppersQueryPower
{
public:
	CommandSteppersQueryPower(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::SteppersQueryPower; }

	std::string ToString()
	{
		std::string cmd = "C 15 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"stepper query resolution"
//}
class CommandStepperQueryResolution
{
public:
	CommandStepperQueryResolution(unsigned long commandId)
	{
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperQueryResolution; }

	std::string ToString()
	{
		std::string cmd = "C 50 " + std::to_string(_commandId & 0xffff);
		return cmd;
	}

private:
	unsigned long _commandId;
};

//{
//	"command":"stepper config step",
//  "index":0
//	"highClks":1,
//	"lowClks":1
//}
class CommandStepperConfigStep
{
private:
	int _stepperIndex;
	int _highClks;
	int _lowClks;
	unsigned long _commandId;

public:
	CommandStepperConfigStep(int stepperIndex, int lowClks, int highClks, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_highClks = highClks;
		_lowClks = lowClks;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperConfigStep; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 51 %d %d %d %d"), _stepperIndex, _lowClks, _highClks, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper acceleration buffer",
//	"index":0,
//	"value":5
//}
class CommandStepperAccelerationBuffer
{
private:
	int _stepperIndex;
	int _buffer;
	unsigned long _commandId;

public:
	CommandStepperAccelerationBuffer(int stepperIndex, int buffer, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_buffer = buffer;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperAccelerationBuffer; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 52 %d %d %d"), _stepperIndex, _buffer, _commandId);
		return cmd;
	}
};

//{
//	"command":"stepper acceleration buffer decrement",
//	"index":0,
//	"value":1
//}
class CommandStepperAccelerationBufferDecrement
{
private:
	int _stepperIndex;
	int _decrement;
	unsigned long _commandId;

public:
	CommandStepperAccelerationBufferDecrement(int stepperIndex, int decrement, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_decrement = decrement;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperAccelerationBufferDecrement; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 53 %d %d %d"), _stepperIndex, _decrement, _commandId);
		return cmd;
	}
};

//{
//	"command":"stepper deceleration buffer",
//	"index":0,
//	"value":5
//}
class CommandStepperDecelerationBuffer
{
private:
	int _stepperIndex;
	int _buffer;
	unsigned long _commandId;

public:
	CommandStepperDecelerationBuffer(int stepperIndex, int buffer, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_buffer = buffer;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperDecelerationBuffer; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 54 %d %d %d"), _stepperIndex, _buffer, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper deceleration buffer increment",
//	"index":0,
//	"value":1
//}
class CommandStepperDecelerationBufferIncrement
{
private:
	int _stepperIndex;
	int _increment;
	unsigned long _commandId;

public:
	CommandStepperDecelerationBufferIncrement(int stepperIndex, int increment, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_increment = increment;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperDecelerationBufferIncrement; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 55 %d %d %d"), _stepperIndex, _increment, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper enable",
//	"index":0,
//	"enable":true
//}
class CommandStepperEnable
{
private:
	int _stepperIndex;
	bool _enable;
	unsigned long _commandId;

public:
	CommandStepperEnable(int stepperIndex, bool enable, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_enable = enable;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperEnable; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 56 %d %d %d"), _stepperIndex, _enable ? 1 : 0, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper forward",
//	"index":0,
//	"forward":true
//}
class CommandStepperForward
{
private:
	int _stepperIndex;
	bool _forward;
	unsigned long _commandId;

public:
	CommandStepperForward(int stepperIndex, bool forward, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_forward = forward;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperForward; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 57 %d %d %d"), _stepperIndex, _forward ? 1 : 0, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper steps",
//	"index":0,
//	"value":20
//}
class CommandStepperSteps
{
private:
	int _stepperIndex;
	int _steps;
	unsigned long _commandId;

public:
	CommandStepperSteps(int stepperIndex, int steps, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_steps = steps;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperSteps; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 58 %d %d %d"), _stepperIndex, _steps, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper run"
//}
class CommandStepperRun
{
public:
	CommandStepperRun(unsigned int stepperIndex, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperRun; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 59 %d %d"), _stepperIndex, _commandId & 0xffff);
		return cmd;
	}

private:
	unsigned int _stepperIndex;
	unsigned long _commandId;
};

//{
//	"command":"stepper config home",
//	"index":0,
//	"locatorIndex":0,
//	"lineNumberStart":0,
//	"lineNumberTerminal":1
//}
class CommandStepperConfigHome
{
private:
	int _stepperIndex;
	int _locatorIndex;
	int _lineNumberStart;
	int _lineNumberTerminal;
	unsigned long _commandId;

public:
	CommandStepperConfigHome(int stepperIndex, int locatorIndex, int lineNumberStart, int lineNumberTerminal, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_locatorIndex = locatorIndex;
		_lineNumberStart = lineNumberStart;
		_lineNumberTerminal = lineNumberTerminal;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperConfigHome; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 60 %d %d %d %d %d"),
				_stepperIndex,
				_locatorIndex,
				_lineNumberStart,
				_lineNumberTerminal,
				_commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"stepper query",
//	"index":0
//}
class CommandStepperQuery
{
private:
	int _stepperIndex;
	unsigned long _commandId;

public:
	CommandStepperQuery(int stepperIndex, unsigned long commandId)
	{
		_stepperIndex = stepperIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::StepperQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 61 %d %d"), _stepperIndex, _commandId & 0xffff);
		return cmd;
	}
};

//{
//	"command":"locator query",
//	"index":0
//}
class CommandLocatorQuery
{
private:
	int _locatorIndex;
	unsigned long _commandId;

public:
	CommandLocatorQuery(int locatorIndex, unsigned long commandId)
	{
		_locatorIndex = locatorIndex;
		_commandId = commandId;
	}

	CommandType Type() { return CommandType::LocatorQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 100 %d %d"), _locatorIndex, _commandId & 0xffff);
		return cmd;
	}
};

// translate JSON command to device command
class CommandTranslator
{
public:
	CommandTranslator(std::string jsonCmd);

	CommandType Type();
	std::string JsonCommand();

	std::shared_ptr<CommandDevicesGet> GetCommandDevicesGet();
	std::shared_ptr<CommandDeviceConnect> GetCommandDeviceConnect();
	std::shared_ptr<CommandDeviceQueryPower> GetCommandDeviceQueryPower();
	std::shared_ptr<CommandDeviceQueryFuse> GetCommandDeviceQueryFuse();
	std::shared_ptr<CommandBdcsPowerOn> GetCommandBdcsPowerOn();
	std::shared_ptr<CommandBdcsPowerOff> GetCommandBdcsPowerOff();
	std::shared_ptr<CommandBdcsQueryPower> GetCommandBdcsQueryPower();
	std::shared_ptr<CommandBdcCoast> GetCommandBdcCoast();
	std::shared_ptr<CommandBdcReverse> GetCommandBdcReverse();
	std::shared_ptr<CommandBdcForward> GetCommandBdcForward();
	std::shared_ptr<CommandBdcBreak> GetCommandBdcBreak();
	std::shared_ptr<CommandBdcQuery> GetCommandBdcQuery();
	std::shared_ptr<CommandSteppersPowerOn> GetCommandSteppersPowerOn();
	std::shared_ptr<CommandSteppersPowerOff> GetCommandSteppersPowerOff();
	std::shared_ptr<CommandSteppersQueryPower> GetCommandSteppersQueryPower();
	std::shared_ptr<CommandStepperQueryResolution> GetCommandStepperQueryResolution();
	std::shared_ptr<CommandStepperConfigStep> GetCommandStepperConfigStep();
	std::shared_ptr<CommandStepperAccelerationBuffer> GetCommandStepperAccelerationBuffer();
	std::shared_ptr<CommandStepperAccelerationBufferDecrement> GetCommandStepperAccelerationBufferDecrement();
	std::shared_ptr<CommandStepperDecelerationBuffer> GetCommandStepperDecelerationBuffer();
	std::shared_ptr<CommandStepperDecelerationBufferIncrement> GetCommandStepperDecelerationBufferIncrement();
	std::shared_ptr<CommandStepperEnable> GetCommandStepperEnable();
	std::shared_ptr<CommandStepperForward> GetCommandStepperForward();
	std::shared_ptr<CommandStepperSteps> GetCommandStepperSteps();
	std::shared_ptr<CommandStepperRun> GetCommandStepperRun();
	std::shared_ptr<CommandStepperConfigHome> GetCommandStepperConfigHome();
	std::shared_ptr<CommandStepperQuery> GetCommandStepperQuery();
	std::shared_ptr<CommandLocatorQuery> GetCommandLocatorQuery();

private:
	std::string _jsonCmd;
	CommandType _type;

	const std::string strCommandDevicesGet = "devices get";
	const std::string strCommandDeviceConnect = "device connect";
	const std::string strCommandDeviceQueryPower = "device query power";
	const std::string strCommandDeviceQueryFuse = "device query fuse";
	const std::string strCommandBdcsPowerOn = "bdcs power on";
	const std::string strCommandBdcsPowerOff = "bdcs power off";
	const std::string strCommandBdcsQueryPower = "bdcs query power";
	const std::string strCommandBdcCoast = "bdc coast";
	const std::string strCommandBdcReverse = "bdc reverse";
	const std::string strCommandBdcForward = "bdc forward";
	const std::string strCommandBdcBreak = "bdc break";
	const std::string strCommandBdcQuery = "bdc query";
	const std::string strCommandSteppersPowerOn = "steppers power on";
	const std::string strCommandSteppersPowerOff = "steppers power off";
	const std::string strCommandSteppersQueryPower = "steppers query power";
	const std::string strCommandStepperQueryResolution = "stepper query resolution";
	const std::string strCommandStepperConfigStep = "stepper config step";
	const std::string strCommandStepperAccelerationBuffer = "stepper acceleration buffer";
	const std::string strCommandStepperAccelerationBufferDecrement = "stepper acceleration buffer decrement";
	const std::string strCommandStepperDecelerationBuffer = "stepper deceleration buffer";
	const std::string strCommandStepperDecelerationBufferIncrement = "stepper deceleration buffer increment";
	const std::string strCommandStepperEnable = "stepper enable";
	const std::string strCommandStepperForward = "stepper forward";
	const std::string strCommandStepperSteps = "stepper steps";
	const std::string strCommandStepperRun = "stepper run";
	const std::string strCommandStepperConfigHome = "stepper config home";
	const std::string strCommandStepperQuery = "stepper query";
	const std::string strCommandLocatorQuery = "locator query";
};

#endif /* COMMANDPARSER_H_ */
