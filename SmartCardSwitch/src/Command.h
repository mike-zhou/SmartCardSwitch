/*
 * Command.h
 *
 *  Created on: Oct 14, 2018
 *      Author: user1
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>

class Command
{
public:
	Command()
	{
		_commandId = _commandIdSeed++;
		_commandUndoId = _commandIdSeed++;
	}
	virtual ~Command() {}

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

class CommandDevicesGet: public Command
{
public:
	virtual std::string ToCommand() override;
};

class CommandDeviceConnect: public Command
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

class CommandDeviceQueryPower: public Command
{
public:
	virtual std::string ToCommand() override;
};

class CommandBdcsPowerOn: public Command
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsPowerOff: public Command
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsQueryPower: public Command
{
public:
	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcOperation: public Command
{
public:
	enum BdcMode
	{
		COAST = 0,
		REVERSE,
		FORWARD,
		BREAK
	};

	CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode, unsigned long delayMs);

	virtual std::string GetUndoState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	unsigned int _bdcIndex;
	enum BdcMode _undoMode;
	enum BdcMode _finalMode;
	unsigned long _delayMs; //milliseconds to delay before result is returned.
};

class CommandStepperQueryClkPeriod: public Command
{
public:
	virtual std::string ToCommand() override;
};

class CommandStepperConfigStep: public Command
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

class CommandStepperAccelerationBuffer: public Command
{
public:
	CommandStepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperAccelerationBufferDecrement: public Command
{
public:
	CommandStepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBuffer: public Command
{
public:
	CommandStepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBufferIncrement: public Command
{
public:
	CommandStepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string ToCommand() override;
	virtual std::string GetFinalState() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperEnable: public Command
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

class CommandStepperConfigHome: public Command
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

class CommandStepperMove: public Command
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
