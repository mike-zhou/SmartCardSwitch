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

	virtual std::string GetInitialState() { std::string empty; return empty; }
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
	CommandDevicesGet() {}
	~CommandDevicesGet() {}

	virtual std::string ToCommand() override;
};

class CommandDeviceConnect: public Command
{
public:
	CommandDeviceConnect(const std::string& deviceName);
	~CommandDeviceConnect() {}

	virtual std::string GetInitialState() override;
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
	CommandBdcsPowerOn() {}
	~CommandBdcsPowerOn() {}

	virtual std::string GetInitialState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsPowerOff: public Command
{
public:
	CommandBdcsPowerOff() {}
	~CommandBdcsPowerOff() {}

	virtual std::string GetInitialState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;
};

class CommandBdcsQueryPower: public Command
{
public:
	CommandBdcsQueryPower() {}
	~CommandBdcsQueryPower() {}

	virtual std::string GetInitialState() override;
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

	CommandBdcOperation(unsigned int bdcIndex, BdcMode initialMode, BdcMode finalMode, unsigned long delayMs);
	~CommandBdcOperation() {}

	virtual std::string GetInitialState() override;
	virtual std::string GetFinalState() override;

	virtual std::string ToCommand() override;
	virtual std::string ToCommandUndo() override;

private:
	unsigned int _bdcIndex;
	enum BdcMode _initialMode;
	enum BdcMode _finalMode;
	unsigned long _delayMs; //millisecond.
};

class CommandStepperQueryClkPeriod: public Command
{
public:
	virtual std::string ToCommand() override;
};

#endif /* COMMAND_H_ */
