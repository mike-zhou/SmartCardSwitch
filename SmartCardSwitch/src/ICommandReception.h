/*
 * ICommandReception.h
 *
 *  Created on: Nov 13, 2018
 *      Author: mikez
 */

#ifndef ICOMMANDRECEPTION_H_
#define ICOMMANDRECEPTION_H_

#include <string>

#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

class ICommandDataTypes
{
public:

	typedef unsigned long CommandId;

	const CommandId InvalidCommandId = 0;

	enum class BdcStatus
	{
		UNKNOWN,
		COAST,
		REVERSE,
		FORWARD,
		BREAK
	};

	enum class StepperState
	{
		Unknown = 0,
		ApproachingHomeLocator,
		LeavingHomeLocator,
		GoingHome,
		KnownPosition,
		Accelerating,
		Cruising,
		Decelerating
	};

};

class IResponseReceiver: public ICommandDataTypes
{
public:
	virtual ~IResponseReceiver() {}

	virtual void OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices) {}
	virtual void OnDeviceConnect(CommandId key, bool bSuccess)  {}
	virtual void OnDeviceDelay(CommandId key, bool bSuccess)  {}
	virtual void OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)  {}
	virtual void OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn)  {}
	virtual void OnDeviceQueryHomeState(CommandId key, bool homePositioned) {}
	virtual void OnDeviceGoHome(CommandId key, bool bSuccess) {}
	virtual void OnOptPowerOn(CommandId key, bool bSuccess)  {}
	virtual void OnOptPowerOff(CommandId key, bool bSuccess)  {}
	virtual void OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)  {}
	virtual void OnDcmPowerOn(CommandId key, bool bSuccess)  {}
	virtual void OnDcmPowerOff(CommandId key, bool bSuccess)  {}
	virtual void OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered)  {}
	virtual void OnBdcsPowerOn(CommandId key, bool bSuccess) {}
	virtual void OnBdcsPowerOff(CommandId key, bool bSuccess) {}
	virtual void OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered) {}
	virtual void OnBdcCoast(CommandId key, bool bSuccess) {}
	virtual void OnBdcReverse(CommandId key, bool bSuccess) {}
	virtual void OnBdcForward(CommandId key, bool bSuccess) {}
	virtual void OnBdcBreak(CommandId key, bool bSuccess) {}
	virtual void OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status) {}
	virtual void OnSteppersPowerOn(CommandId key, bool bSuccess) {}
	virtual void OnSteppersPowerOff(CommandId key, bool bSuccess) {}
	virtual void OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered) {}
	virtual void OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs) {}
	virtual void OnStepperConfigStep(CommandId key, bool bSuccess) {}
	virtual void OnStepperAccelerationBuffer(CommandId key, bool bSuccess) {}
	virtual void OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess) {}
	virtual void OnStepperDecelerationBuffer(CommandId key, bool bSuccess) {}
	virtual void OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess) {}
	virtual void OnStepperEnable(CommandId key, bool bSuccess) {}
	virtual void OnStepperForward(CommandId key, bool bSuccess) {}
	virtual void OnStepperSteps(CommandId key, bool bSuccess) {}
	virtual void OnStepperRun(CommandId key, bool bSuccess) {}
	virtual void OnStepperConfigHome(CommandId key, bool bSuccess) {}
	virtual void OnStepperMove(CommandId key, bool bSuccess) {}
	virtual void OnStepperQuery(CommandId key, bool bSuccess,
								StepperState state,
								bool bEnabled,
								bool bForward,
								bool bForwardClockwise,
								unsigned int locatorIndex,
								unsigned int locatorLineNumberStart,
								unsigned int locatorLineNumberTerminal,
								unsigned long homeOffset,
								unsigned long lowClks,
								unsigned long highClks,
								unsigned long accelerationBuffer,
								unsigned long accelerationBufferDecrement,
								unsigned long decelerationBuffer,
								unsigned long decelerationBufferIncrement) {}

	virtual void OnStepperSetState(CommandId key, bool bSuccess) {}
	virtual void OnStepperForwardClockwise(CommandId key, bool bSuccess) {}
	virtual void OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput) {}
	virtual void OnSaveMovementConfig(CommandId key, bool bSuccess) {}
	virtual void OnSaveCoordinates(CommandId key, bool bSuccess) {}
};

class ICommandReception: public ICommandDataTypes
{
public:
	virtual ~ICommandReception() {}

	virtual CommandId DevicesGet() = 0;
	virtual CommandId DeviceConnect(unsigned int index) = 0;
	virtual CommandId DeviceDelay(unsigned int clks) = 0;
	virtual CommandId DeviceQueryPower() = 0;
	virtual CommandId DeviceQueryFuse() = 0;
	virtual CommandId OptPowerOn() = 0;
	virtual CommandId OptPowerOff() = 0;
	virtual CommandId OptQueryPower() = 0;
	virtual CommandId DcmPowerOn(unsigned int index) = 0;
	virtual CommandId DcmPowerOff(unsigned int index) = 0;
	virtual CommandId DcmQueryPower(unsigned int index) = 0;
	virtual CommandId BdcsPowerOn() = 0;
	virtual CommandId BdcsPowerOff() = 0;
	virtual CommandId BdcsQueryPower() = 0;
	virtual CommandId BdcCoast(unsigned int index) = 0;
	virtual CommandId BdcReverse(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles) = 0;
	virtual CommandId BdcForward(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles) = 0;
	virtual CommandId BdcBreak(unsigned int index) = 0;
	virtual CommandId BdcQuery(unsigned int index) = 0;
	virtual CommandId SteppersPowerOn() = 0;
	virtual CommandId SteppersPowerOff() = 0;
	virtual CommandId SteppersQueryPower() = 0;
	virtual CommandId StepperQueryResolution() = 0;
	virtual CommandId StepperConfigStep(unsigned int index, unsigned short lowClks, unsigned short highClks) = 0;
	virtual CommandId StepperAccelerationBuffer(unsigned int index, unsigned short value) = 0;
	virtual CommandId StepperAccelerationBufferDecrement(unsigned int index, unsigned short value) = 0;
	virtual CommandId StepperDecelerationBuffer(unsigned int index, unsigned short value) = 0;
	virtual CommandId StepperDecelerationBufferIncrement(unsigned int index, unsigned short value) = 0;
	virtual CommandId StepperEnable(unsigned int index, bool bEnable) = 0;
	virtual CommandId StepperForward(unsigned int index, bool forward) = 0;
	virtual CommandId StepperSteps(unsigned int index, unsigned short value) = 0;
	virtual CommandId StepperRun(unsigned int index, unsigned short intialPos, unsigned short finalPos) = 0;
	virtual CommandId StepperConfigHome(unsigned int index, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal) = 0;
	virtual CommandId StepperMove(unsigned int index, unsigned short steps) = 0;
	virtual CommandId StepperSetState(unsigned int index, StepperState state) = 0;
	virtual CommandId StepperForwardClockwise(unsigned int index, bool bForwardClockwise) = 0;
	virtual CommandId StepperQuery(unsigned int index) = 0;
	virtual CommandId LocatorQuery(unsigned int index) = 0;
	virtual CommandId SaveMovementConfig() = 0;

	virtual void AddResponseReceiver(IResponseReceiver * p) = 0;
};



#endif /* ICOMMANDRECEPTION_H_ */
