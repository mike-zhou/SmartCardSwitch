/*
 * Command.h
 *
 *  Created on: Oct 14, 2018
 *      Author: user1
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <string>

//base of all commands which are to be sent to DeviceAccessor object.
class DeviceCommand
{
public:

	DeviceCommand()
	{
		_commandId = _commandIdSeed++;
	}

	virtual ~DeviceCommand() {}

	virtual std::string CommandKey() = 0;
	virtual std::string ToJsonCommandString() = 0;
	unsigned long CommandId() { return _commandId; }

private:
	static unsigned long _commandIdSeed;

	unsigned long _commandId;
};

class CommandDevicesGet: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandDeviceConnect: public DeviceCommand
{
public:
	CommandDeviceConnect(const std::string& deviceName);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	std::string _deviceName;
};

class CommandDeviceDelay: public DeviceCommand
{
public:
	CommandDeviceDelay(unsigned int clks);
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _clks;
};

class CommandDeviceQueryPower: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandDeviceQueryFuse: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandBdcsPowerOn: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandBdcsPowerOff: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandBdcsQueryPower: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;

	virtual std::string ToJsonCommandString() override;
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

	CommandBdcOperation(unsigned int bdcIndex, BdcMode undoMode, BdcMode finalMode, unsigned int lowClks, unsigned int highClks, unsigned int cycles);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _bdcIndex;
	enum BdcMode _undoMode;
	enum BdcMode _finalMode;
	unsigned int _lowClks;
	unsigned int _highClks;
	unsigned int _cycles;
};

class CommandBdcQuery: public DeviceCommand
{
public:
	CommandBdcQuery(unsigned int bdcIndex);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _bdcIndex;
};

class CommandSteppersPowerOn: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandSteppersPowerOff: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandSteppersQueryPower: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandStepperQueryResolution: public DeviceCommand
{
public:
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandStepperConfigStep: public DeviceCommand
{
public:
	CommandStepperConfigStep(unsigned int stepperIndex, unsigned long lowClks, unsigned long highClks);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _lowClks;
	unsigned long _highClks;
};

class CommandStepperAccelerationBuffer: public DeviceCommand
{
public:
	CommandStepperAccelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperAccelerationBufferDecrement: public DeviceCommand
{
public:
	CommandStepperAccelerationBufferDecrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBuffer: public DeviceCommand
{
public:
	CommandStepperDecelerationBuffer(unsigned int stepperIndex, unsigned long value);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperDecelerationBufferIncrement: public DeviceCommand
{
public:
	CommandStepperDecelerationBufferIncrement(unsigned int stepperIndex, unsigned long value);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _value;
};

class CommandStepperEnable: public DeviceCommand
{
public:
	CommandStepperEnable(unsigned int stepperIndex, bool enable);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	bool _enable;
};

class CommandStepperForward: public DeviceCommand
{
public:
	CommandStepperForward(unsigned int stepperIndex, bool bForward);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	bool _bForward;
};

class CommandStepperSteps: public DeviceCommand
{
public:
	CommandStepperSteps(unsigned int stepperIndex, unsigned long steps);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _steps;
};

class CommandStepperRun: public DeviceCommand
{
public:
	CommandStepperRun(unsigned int stepperIndex, unsigned long initialPosition, unsigned long finalPosition);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _initialPosition;
	unsigned long _finalPosition;
};

class CommandStepperConfigHome: public DeviceCommand
{
public:
	CommandStepperConfigHome(unsigned int stepperIndex,
							unsigned int locatorIndex,
							unsigned int lineNumberStart,
							unsigned int lineNumberTerminal);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned int _locatorIndex;
	unsigned int _lineNumberStart;
	unsigned int _lineNumberTerminal;
};

class CommandStepperQuery: public DeviceCommand
{
public:
	CommandStepperQuery(unsigned int stepperIndex);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
};

class CommandStepperSetState: public DeviceCommand
{
public:
	CommandStepperSetState(unsigned int stepperIndex, unsigned int state);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned int _state;
};

class CommandStepperMove: public DeviceCommand
{
public:
	CommandStepperMove(unsigned int stepperIndex, unsigned long position, bool forward, unsigned long steps);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	unsigned long _position;
	bool _forward;
	unsigned long _steps;
};

class CommandStepperForwardClockwise: public DeviceCommand
{
public:
	CommandStepperForwardClockwise(unsigned int stepperIndex, bool forwardClockwise);

	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _stepperIndex;
	bool _forwardClockwise;
};

class CommandLocatorQuery: public DeviceCommand
{
public:
	CommandLocatorQuery(unsigned int locatorIndex);
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _locatorIndex;
};

class CommandOptPowerOn: public DeviceCommand
{
public:
	CommandOptPowerOn();
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandOptPowerOff: public DeviceCommand
{
public:
	CommandOptPowerOff();
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandOptQueryPower: public DeviceCommand
{
public:
	CommandOptQueryPower();
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;
};

class CommandDcmPowerOn: public DeviceCommand
{
public:
	CommandDcmPowerOn(unsigned int index);
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _index;
};

class CommandDcmPowerOff: public DeviceCommand
{
public:
	CommandDcmPowerOff(unsigned int index);
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _index;
};

class CommandDcmQueryPower: public DeviceCommand
{
public:
	CommandDcmQueryPower(unsigned int index);
	virtual std::string CommandKey() override;
	virtual std::string ToJsonCommandString() override;

private:
	unsigned int _index;
};

#endif /* COMMAND_H_ */
