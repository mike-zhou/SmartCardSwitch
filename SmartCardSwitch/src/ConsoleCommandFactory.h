/*
 * ConsoleCommandFactory.h
 *
 *  Created on: Nov 20, 2018
 *      Author: mikez
 */

#ifndef CONSOLECOMMANDFACTORY_H_
#define CONSOLECOMMANDFACTORY_H_

#include <string>

class ConsoleCommandFactory
{
public:

	enum class Type
	{
		Invalid = -1,
		DevicesGet = 0,
		DeviceConnect = 1,
		DeviceQueryPower = 2,
		DeviceQueryFuse = 3,
		DeviceDelay = 4,
		OptPowerOn = 20,
		OptPowerOff = 21,
		OptQueryPower = 22,
		DcmPowerOn = 30,
		DcmPowerOff = 31,
		DcmQueryPower =32,
		BdcsPowerOn = 40,
		BdcsPowerOff = 41,
		BdcsQueryPower = 42,
		BdcCoast = 43,
		BdcReverse = 44,
		BdcForward = 45,
		BdcBreak = 46,
		BdcQuery = 47,
		SteppersPowerOn = 60,
		SteppersPowerOff = 61,
		SteppersQueryPower = 62,
		StepperQueryResolution = 63,
		StepperConfigStep = 64,
		StepperAccelerationBuffer = 65,
		StepperAccelerationBufferDecrement = 66,
		StepperDecelerationBuffer = 67,
		StepperDecelerationBufferIncrement = 68,
		StepperEnable = 69,
		StepperForward = 70,
		StepperSteps = 71,
		StepperRun = 72,
		StepperConfigHome = 73,
		StepperMove = 74,
		StepperQuery = 75,
		StepperSetState = 76,
		LocatorQuery = 90,
		SaveMovementConfig = 300,
		SaveMovementConfigStepperBoundary = 301,
		SaveMovementConfigStepperGeneral = 302,
		SaveMovementConfigCardInsert = 303,
		SaveMovementConfigGoHome = 304,
		SaveMovementConfigBdc = 305,
		LoadMovementConfigStepper = 320,
		SaveCoordinates = 350,
		SaveCoordinateSmartCardPlaceStartZ = 351,
		SaveCoordinateSmartCardFetchOffset = 352,
		SaveCoordinateSmartCardReleaseOffsetZ = 353,
		SaveCoordinateSmartCardReaderSlowInsertEndY = 354
	};

	static std::string GetHelp();

	static std::string CmdDevicesGet() 						{ return std::string("0\r\n"); }
	static std::string CmdDeviceConnect(unsigned int index) { return "1 " + std::to_string(index) + "\r\n"; }
	static std::string CmdDeviceQueryPower() 				{ return std::string("2 \r\n"); }
	static std::string CmdDeviceQueryFuse() 				{ return std::string("3 \r\n"); }
	static std::string CmdDeviceDelay(unsigned int clks)	{ return std::string("4 ") + std::to_string(clks) + "\r\n"; }
	static std::string CmdOptPowerOn() 						{ return std::string("20 \r\n"); }
	static std::string CmdOptPowerOff() 					{ return std::string("21 \r\n"); }
	static std::string CmdOptQueryPower() 					{ return std::string("22 \r\n"); }
	static std::string CmdDcmPowerOn(unsigned int index) 	{ return std::string("30 ") + std::to_string(index) + "\r\n"; }
	static std::string CmdDcmPowerOff(unsigned int index) 	{ return std::string("31 ") + std::to_string(index) + "\r\n"; }
	static std::string CmdDcmQueryPower() 					{ return std::string("32 \r\n"); }
	static std::string CmdBdcsPowerOn() 					{ return std::string("40 \r\n"); }
	static std::string CmdBdcsPowerOff() 					{ return std::string("41 \r\n"); }
	static std::string CmdBdcsQueryPower() 					{ return std::string("42 \r\n"); }
	static std::string CmdBdcCoast(unsigned int index) 		{ return "43 " + std::to_string(index) + "\r\n"; }

	static std::string CmdBdcReverse(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles)
															{ return "44 " + std::to_string(index) + " " + std::to_string(lowClks) + " " + std::to_string(highClks) + " " + std::to_string(cycles) + "\r\n"; }

	static std::string CmdBdcForward(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles)
															{ return "45 " + std::to_string(index) + " " + std::to_string(lowClks) + " " + std::to_string(highClks) + " " + std::to_string(cycles) + "\r\n"; }

	static std::string CmdBdcBreak(unsigned int index) 		{ return "46 " + std::to_string(index) + "\r\n"; }
	static std::string CmdBdcQuery(unsigned int index) 		{ return "47 " + std::to_string(index) + "\r\n"; }
	static std::string CmdSteppersPowerOn() 				{ return std::string("60 \r\n"); }
	static std::string CmdSteppersPowerOff() 				{ return std::string("61 \r\n"); }
	static std::string CmdSteppersQueryPower() 				{ return std::string("62 \r\n"); }
	static std::string CmdStepperQueryresolution() 			{ return std::string("63 \r\n"); }

	static std::string CmdStepperConfigStep(
						unsigned int index,
						unsigned int lowClks,
						unsigned int highClks)
	{
		return "64 " + std::to_string(index) + " " + std::to_string(lowClks) + " " + std::to_string(highClks) + "\r\n";
	}

	static std::string CmdStepperAccelerationBuffer(unsigned int index, unsigned long value)
	{
		return "65 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string CmdStepperAccelerationBufferDecrement(unsigned int index, unsigned long value)
	{
		return "66 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string CmdStepperDecelerationBuffer(unsigned int index, unsigned long value)
	{
		return "67 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string CmdStepperDecelerationBufferIncrement(unsigned int index, unsigned long value)
	{
		return "68 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string CmdStepperEnable(unsigned int index, bool enable)
	{
		if(enable)
			return "69 " + std::to_string(index) + " 1 \r\n";
		else
			return "69 " + std::to_string(index) + " 0 \r\n";
	}

	static std::string CmdStepperForward(unsigned int index, bool forward)
	{
		if(forward)
			return "70 " + std::to_string(index) + " 1 \r\n";
		else
			return "70 " + std::to_string(index) + " 0 \r\n";
	}

	static std::string CmdStepperSteps(unsigned int index, unsigned long value)
	{
		return "71 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string CmdStepperRun(unsigned int index, unsigned long initialPos, unsigned long finalPos)
	{
		return "72 " + std::to_string(index) + " " + std::to_string(initialPos) + " " + std::to_string(finalPos) + "\r\n";
	}

	static std::string CmdStepperConfigHome(unsigned int index,
										unsigned int locatorIndex,
										unsigned int locatorLineNumberStart,
										unsigned int locatorLineNumberTerminal)
	{
		return "73 " + std::to_string(index) + " " + std::to_string(locatorIndex) + " " + std::to_string(locatorLineNumberStart) + " " + std::to_string(locatorLineNumberTerminal) + "\r\n";
	}

	static std::string CmdStepperQuery(unsigned int index) 	{ return "75 " + std::to_string(index) + "\r\n"; }
	static std::string CmdLocatorQuery(unsigned int index) 	{ return "90 " + std::to_string(index) + "\r\n"; }

	static Type GetCmdType(const std::string& consoleCmd);
	static bool GetParameterStepperIndex(const std::string & consoleCmd, unsigned int & stepperIndex);
	static bool GetParameterStepperSteps(const std::string & consoleCmd, unsigned int & steps);
	static bool GetParameterStepperForward(const std::string & consoleCmd, bool & bForward);
	static bool GetParameterLocatorIndex(const std::string & consoleCmd, unsigned int & locatorIndex);
};




#endif /* CONSOLECOMMANDFACTORY_H_ */
