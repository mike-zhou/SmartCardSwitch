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
private:
	unsigned long _commandId;

public:
	CommandDevicesGet(unsigned long commandId) { _commandId = commandId; }

	CommandType Type() { return CommandType::DevicesGet; }
	unsigned long CommandId() { return _commandId; }

	std::string ToString()
	{
		std::string empty;
		return empty;
	}
};

//{
//	"command":"device connect",
//	"device": "device12345"
//}
class CommandDeviceConnect
{
private:
	std::string _deviceName;
	unsigned long _commandId;

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
	CommandType Type() { return CommandType::BdcsPowerOn; }

	std::string ToString()
	{
		std::string cmd = "C 40";
		return cmd;
	}
};

//{
//	"command":"bdcs power off"
//}
class CommandBdcsPowerOff
{
public:
	CommandType Type() { return CommandType::BdcsPowerOff; }

	std::string ToString()
	{
		std::string cmd = "C 41";
		return cmd;
	}
};

//{
//	"command":"bdcs query power"
//}
class CommandBdcsQueryPower
{
public:
	CommandType Type() { return CommandType::BdcsQueryPower; }

	std::string ToString()
	{
		std::string cmd = "C 42";
		return cmd;
	}
};

//{
//	"command":"bdc coast",
//	"index":0
//}
class CommandBdcCoast
{
public:
	CommandBdcCoast(int bdcIndex) { this->bdcIndex = bdcIndex; }

	CommandType Type() { return CommandType::BdcCoast; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 43 %d"), bdcIndex);
		return cmd;
	}

private:
	int bdcIndex;
};

//{
//	"command":"bdc reverse",
//	"index":0
//}
class CommandBdcReverse
{
public:
	CommandBdcReverse(int bdcIndex) { this->bdcIndex = bdcIndex; }

	CommandType Type() { return CommandType::BdcReverse; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 44 %d"), bdcIndex);
		return cmd;
	}

private:
	int bdcIndex;
};

//{
//	"command":"bdc forward",
//	"index":0
//}
class CommandBdcForward
{
public:
	CommandBdcForward(int bdcIndex) { this->bdcIndex = bdcIndex; }

	CommandType Type() { return CommandType::BdcForward; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 45 %d"), bdcIndex);
		return cmd;
	}

private:
	int bdcIndex;
};

//{
//	"command":"bdc break",
//	"index":0
//}
class CommandBdcBreak
{
public:
	CommandBdcBreak(int bdcIndex) { this->bdcIndex = bdcIndex; }

	CommandType Type() { return CommandType::BdcBreak; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 46 %d"), bdcIndex);
		return cmd;
	}

private:
	int bdcIndex;
};

//{
//	"command":"bdc query",
//	"index":0
//}
class CommandBdcQuery
{
public:
	CommandBdcQuery(int bdcIndex) { this->bdcIndex = bdcIndex; }

	CommandType Type() { return CommandType::BdcQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 47 %d"), bdcIndex);
		return cmd;
	}

private:
	int bdcIndex;
};

//{
//	"command":"steppers power on"
//}
class CommandSteppersPowerOn
{
public:
	CommandType Type() { return CommandType::SteppersPowerOn; }

	std::string ToString()
	{
		std::string cmd = "C 13";
		return cmd;
	}
};

//{
//	"command":"steppers power off"
//}
class CommandSteppersPowerOff
{
public:
	CommandType Type() { return CommandType::SteppersPowerOff; }

	std::string ToString()
	{
		std::string cmd = "C 14";
		return cmd;
	}
};

//{
//	"command":"steppers query power"
//}
class CommandSteppersQueryPower
{
public:
	CommandType Type() { return CommandType::SteppersQueryPower; }

	std::string ToString()
	{
		std::string cmd = "C 15";
		return cmd;
	}
};

//{
//	"command":"stepper query resolution"
//}
class CommandStepperQueryResolution
{
public:
	CommandType Type() { return CommandType::StepperQueryResolution; }

	std::string ToString()
	{
		std::string cmd = "C 50";
		return cmd;
	}
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
	int stepperIndex;
	int highClks;
	int lowClks;

public:
	CommandStepperConfigStep(int stepperIndex, int lowClks, int highClks)
	{
		this->stepperIndex = stepperIndex;
		this->highClks = highClks;
		this->lowClks = lowClks;
	}

	CommandType Type() { return CommandType::StepperConfigStep; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 51 %d %d %d"), stepperIndex, lowClks, highClks);
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
	int stepperIndex;
	int buffer;

public:
	CommandStepperAccelerationBuffer(int stepperIndex, int buffer)
	{
		this->stepperIndex = stepperIndex;
		this->buffer = buffer;
	}

	CommandType Type() { return CommandType::StepperAccelerationBuffer; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 52 %d %d"), stepperIndex, buffer);
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
	int stepperIndex;
	int decrement;

public:
	CommandStepperAccelerationBufferDecrement(int stepperIndex, int decrement)
	{
		this->stepperIndex = stepperIndex;
		this->decrement = decrement;
	}

	CommandType Type() { return CommandType::StepperAccelerationBufferDecrement; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 53 %d %d"), stepperIndex, decrement);
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
	int stepperIndex;
	int buffer;

public:
	CommandStepperDecelerationBuffer(int stepperIndex, int buffer)
	{
		this->stepperIndex = stepperIndex;
		this->buffer = buffer;
	}

	CommandType Type() { return CommandType::StepperDecelerationBuffer; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 54 %d %d"), stepperIndex, buffer);
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
	int stepperIndex;
	int increment;

public:
	CommandStepperDecelerationBufferIncrement(int stepperIndex, int increment)
	{
		this->stepperIndex = stepperIndex;
		this->increment = increment;
	}

	CommandType Type() { return CommandType::StepperDecelerationBufferIncrement; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 55 %d %d"), stepperIndex, increment);
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
	int stepperIndex;
	bool enable;

public:
	CommandStepperEnable(int stepperIndex, bool enable)
	{
		this->stepperIndex = stepperIndex;
		this->enable = enable;
	}

	CommandType Type() { return CommandType::StepperEnable; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 56 %d %d"), stepperIndex, enable ? 1 : 0);
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
	int stepperIndex;
	bool forward;

public:
	CommandStepperForward(int stepperIndex, bool forward)
	{
		this->stepperIndex = stepperIndex;
		this->forward = forward;
	}

	CommandType Type() { return CommandType::StepperForward; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 57 %d %d"), stepperIndex, forward ? 1 : 0);
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
	int stepperIndex;
	int steps;

public:
	CommandStepperSteps(int stepperIndex, int steps)
	{
		this->stepperIndex = stepperIndex;
		this->steps = steps;
	}

	CommandType Type() { return CommandType::StepperSteps; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 58 %d %d"), stepperIndex, steps);
		return cmd;
	}
};

//{
//	"command":"stepper run"
//}
class CommandStepperRun
{
public:
	CommandType Type() { return CommandType::StepperRun; }

	std::string ToString()
	{
		std::string cmd = "C 59";
		return cmd;
	}
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
	int stepperIndex;
	int locatorIndex;
	int lineNumberStart;
	int lineNumberTerminal;

public:
	CommandStepperConfigHome(int stepperIndex, int locatorIndex, int lineNumberStart, int lineNumberTerminal)
	{
		this->stepperIndex = stepperIndex;
		this->locatorIndex = locatorIndex;
		this->lineNumberStart = lineNumberStart;
		this->lineNumberTerminal = lineNumberTerminal;
	}

	CommandType Type() { return CommandType::StepperConfigHome; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 60 %d %d %d %d"), stepperIndex, locatorIndex, lineNumberStart, lineNumberTerminal);
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
	int stepperIndex;

public:
	CommandStepperQuery(int stepperIndex)
	{
		this->stepperIndex = stepperIndex;
	}

	CommandType Type() { return CommandType::StepperQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 61 %d"), stepperIndex);
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
	int locatorIndex;

public:
	CommandLocatorQuery(int locatorIndex)
	{
		this->locatorIndex = locatorIndex;
	}

	CommandType Type() { return CommandType::LocatorQuery; }

	std::string ToString()
	{
		std::string cmd = Poco::format(std::string("C 100 %d"), locatorIndex);
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
	unsigned long _commandId;

	const std::string strCommandDevicesGet = "devices get";
	const std::string strCommandDeviceConnect = "device connect";
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
