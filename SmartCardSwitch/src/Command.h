/*
 * Command.h
 *
 *  Created on: Oct 14, 2018
 *      Author: user1
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>

class DeviceCommand
{
public:
	DeviceCommand()
	{
		_commandId = _commandIdSeed++;
		_commandUndoId = _commandIdSeed++;
	}
	virtual ~DeviceCommand() {}

	virtual std::string GetUndoState() { std::string empty; return empty; }
	virtual std::string GetFinalState() { std::string empty; return empty; }

	virtual std::string ToCommand() { std::string empty; return empty; }
	unsigned long CommandId() { return _commandId; }

	virtual std::string ToCommandUndo() { std::string empty; return empty; }
	unsigned long CommandUndoId() { return _commandUndoId; }

private:
	static unsigned long _commandIdSeed;

	unsigned long _commandId;
	unsigned long _commandUndoId;
};

class CommandDevicesGet: public DeviceCommand
{
public:
	virtual std::string ToCommand() override;
};

class CommandDeviceConnect: public DeviceCommand
{
public:
	CommandDeviceConnect(const std::string& deviceName);

	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	std::string _deviceName;
};

class CommandDeviceQueryPower: public DeviceCommand
{
public:
	virtual std::string ToCommand() override;
};

class CommandBdcsPowerOn: public DeviceCommand
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsPowerOff: public DeviceCommand
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsQueryPower: public DeviceCommand
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcOperation: public DeviceCommand
{
public:
	enum BdcMode
	{
		COAST = 0,
		REVERSE,
		FORWARD,
		BREAK
	};

	CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode);

	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	unsigned int _bdcIndex;
	enum BdcMode _undoMode;
	enum BdcMode _finalMode;
};

class CommandStepperQueryClkPeriod: public DeviceCommand
{
public:
	virtual std::string ToCommand() override;
};

class CommandStepperConfigStep: public DeviceCommand
{
public:
	CommandStepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _lowClks;
	unsigned long _highClks;
};

class CommandStepperAccelerationBuffer: public DeviceCommand
{
public:
	CommandStepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperAccelerationBufferDecrement: public DeviceCommand
{
public:
	CommandStepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBuffer: public DeviceCommand
{
public:
	CommandStepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBufferIncrement: public DeviceCommand
{
public:
	CommandStepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperEnable: public DeviceCommand
{
public:
	CommandStepperEnable(unsigned int stepperIndex, bool enable);

	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	unsigned int _stepperIndex;
	bool _enable;
};

class CommandStepperConfigHome: public DeviceCommand
{
public:
	CommandStepperConfigHome(unsigned int stepperIndex,
							unsigned int locatorIndex,
							unsigned int lineNumberStart,
							unsigned int lineNumberTerminal);

	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;

private:
	unsigned int _stepperIndex;
	unsigned int _locatorIndex;
	unsigned int _lineNumberStart;
	unsigned int _lineNumberTerminal;
};

class CommandStepperMove: public DeviceCommand
{
public:
	CommandStepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps);

	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	unsigned int _stepperIndex;
	unsigned long _position;
	bool _forward;
	unsigned long _steps;
};

#endif /* COMMAND_H_ */
