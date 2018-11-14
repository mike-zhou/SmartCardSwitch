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


class IResponseReceiver
{
public:
	virtual ~IResponseReceiver() {}

	typedef unsigned long CommandKey;
	const CommandKey InvalidCommandKey = 0;

	virtual void OnDevicesGet(CommandKey key, bool bSuccess, const std::vector<std::string>& devices) {}
	virtual void OnDeviceConnect(CommandKey key, bool bSuccess)  {}
	virtual void OnDeviceQueryPower(CommandKey key, bool bSuccess, bool bPowered)  {}
	virtual void OnDeviceQueryFuse(CommandKey key, bool bSuccess, bool bFuseOn)  {}
	virtual void OnOptPowerOn(CommandKey key, bool bSuccess)  {}
	virtual void OnOptPowerOff(CommandKey key, bool bSuccess)  {}
	virtual void OnOptQueryPower(CommandKey key, bool bSuccess)  {}
	virtual void OnDcmPowerOn(CommandKey key, bool bSuccess)  {}
	virtual void OnDcmPowerOff(CommandKey key, bool bSuccess)  {}
	virtual void OnDcmQueryPower(CommandKey key, bool bSuccess)  {}
	virtual void OnBdcsPowerOn(CommandKey key, bool bSuccess) {}
	virtual void OnBdcsPowerOff(CommandKey key, bool bSuccess) {}
	virtual void OnBdcsQueryPower(CommandKey key, bool bSuccess, bool bPowered) {}
	virtual void OnBdcCoast(CommandKey key, bool bSuccess) {}
	virtual void OnBdcReverse(CommandKey key, bool bSuccess) {}
	virtual void OnBdcForward(CommandKey key, bool bSuccess) {}
	virtual void OnBdcBreak(CommandKey key, bool bSuccess) {}

	enum class BdcStatus
	{
		UNKNOWN,
		COAST,
		REVERSE,
		FORWARD,
		BREAK
	};
	virtual void OnBdcQuery(CommandKey key, bool bSuccess, BdcStatus status) {}
	virtual void OnSteppersPowerOn(CommandKey key, bool bSuccess) {}
	virtual void OnSteppersPowerOff(CommandKey key, bool bSuccess) {}
	virtual void OnSteppersQueryPower(CommandKey key, bool bSuccess, bool bPowered) {}
	virtual void OnStepperQueryResolution(CommandKey key, bool bSuccess, unsigned long resolutionUs) {}
	virtual void OnStepperConfigStep(CommandKey key, bool bSuccess) {}
	virtual void OnStepperAccelerationBuffer(CommandKey key, bool bSuccess) {}
	virtual void OnStepperAccelerationBufferDecrement(CommandKey key, bool bSuccess) {}
	virtual void OnStepperDecelerationBuffer(CommandKey key, bool bSuccess) {}
	virtual void OnStepperDecelerationBufferIncrement(CommandKey key, bool bSuccess) {}
	virtual void OnStepperEnable(CommandKey key, bool bSuccess) {}
	virtual void OnStepperForward(CommandKey key, bool bSuccess) {}
	virtual void OnStepperSteps(CommandKey key, bool bSuccess) {}
	virtual void OnStepperRun(CommandKey key, bool bSuccess) {}
	virtual void OnStepperConfigHome(CommandKey key, bool bSuccess) {}
	virtual void OnStepperMove(CommandKey key, bool bSuccess) {}

	enum class StepperState
	{
		Unknown = 0,
		ApproachingHomeLocator,
		LeavingHomeLocator,
		GoingHome,
		KnowPosition,
		Accelerating,
		Cruising,
		Decelerating
	};
	virtual void OnStepperQuery(CommandKey key, bool bSuccess,
								StepperState state,
								bool bEnabled,
								bool bForward,
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

	virtual void OnLocatorQuery(CommandKey key, bool bSuccess, unsigned int lowInput) {}
	virtual void OnBdcDelay(CommandKey key, bool bSuccess) {}
	virtual void OnSaveMovementConfig(CommandKey key, bool bSuccess) {}
	virtual void OnSaveCoordinates(CommandKey key, bool bSuccess) {}
};

class ICommandReception
{
public:
	virtual ~ICommandReception() {}

	typedef unsigned long CommandKey;
	const CommandKey InvalidCommandKey = 0;

	virtual CommandKey DevicesGet() = 0;
	virtual CommandKey DeviceConnect(unsigned int index) = 0;
	virtual CommandKey DeviceQueryPower() = 0;
	virtual CommandKey DeviceQueryFuse() = 0;
	virtual CommandKey OptPowerOn() = 0;
	virtual CommandKey OptPowerOff() = 0;
	virtual CommandKey OptQueryPower() = 0;
	virtual CommandKey DcmPowerOn() = 0;
	virtual CommandKey DcmPowerOff() = 0;
	virtual CommandKey DcmQueryPower() = 0;
	virtual CommandKey BdcsPowerOn() = 0;
	virtual CommandKey BdcsPowerOff() = 0;
	virtual CommandKey BdcsQueryPower() = 0;
	virtual CommandKey BdcCoast(unsigned int index) = 0;
	virtual CommandKey BdcReverse(unsigned int index) = 0;
	virtual CommandKey BdcForward(unsigned int index) = 0;
	virtual CommandKey BdcBreak(unsigned int index) = 0;
	virtual CommandKey BdcQuery(unsigned int index) = 0;
	virtual CommandKey SteppersPowerOn() = 0;
	virtual CommandKey SteppersPowerOff() = 0;
	virtual CommandKey SteppersQueryPower() = 0;
	virtual CommandKey StepperQueryResolution() = 0;
	virtual CommandKey StepperConfigStep(unsigned int index, unsigned short lowClks, unsigned short highClks) = 0;
	virtual CommandKey StepperAccelerationBuffer(unsigned int index, unsigned short value) = 0;
	virtual CommandKey StepperAccelerationBufferDecrement(unsigned int index, unsigned short value) = 0;
	virtual CommandKey StepperDecelerationBuffer(unsigned int index, unsigned short value) = 0;
	virtual CommandKey StepperDecelerationBufferIncrement(unsigned int index, unsigned short value) = 0;
	virtual CommandKey StepperEnable(unsigned int index, bool bEnable) = 0;
	virtual CommandKey StepperForward(unsigned int index, bool forward) = 0;
	virtual CommandKey StepperSteps(unsigned int index, unsigned short value) = 0;
	virtual CommandKey StepperRun(unsigned int index, unsigned short intialPos, unsigned short finalPos) = 0;
	virtual CommandKey StepperConfigHome(unsigned int index, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal) = 0;
	virtual CommandKey StepperMove(unsigned int index, unsigned short steps) = 0;
	virtual CommandKey StepperQuery(unsigned int index) = 0;
	virtual CommandKey LocatorQuery(unsigned int index) = 0;
	virtual CommandKey BdcDelay(unsigned int index, unsigned int value) = 0;
	virtual CommandKey SaveMovementConfig() = 0;
	virtual CommandKey SaveCoordinates(unsigned int type, unsigned int index) = 0;

	virtual void AddResponseReceiver(IResponseReceiver * p) = 0;
};



#endif /* ICOMMANDRECEPTION_H_ */
