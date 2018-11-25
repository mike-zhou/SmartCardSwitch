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
		BdcDelay = 200,
		SaveMovementConfig = 300,
		LoadMovementConfigStepper = 301,
		SaveCoordinates = 350,
		LoadCoordinates = 351
	};

	static std::string GetHelp()
	{
		std::string help;

		help = help + "DevicesGet:------------------------ " + "0" + "\r\n";
		help = help + "DeviceConnect:--------------------- " + "1 deviceNumber" + "\r\n";
		help = help + "DeviceQueryPower:------------------ " + "2" + "\r\n";
		help = help + "DeviceQueryFuse:------------------- " + "3" + "\r\n";
		help = help + "OptPowerOn:------------------------ " + "20" + "\r\n";
		help = help + "OptPowerOff:----------------------- " + "21" + "\r\n";
		help = help + "OptQueryPower:--------------------- " + "22" + "\r\n";
		help = help + "DcmPowerOn:------------------------ " + "30" + "\r\n";
		help = help + "DcmPowerOff:----------------------- " + "31" + "\r\n";
		help = help + "DcmQueryPower:--------------------- " + "32" + "\r\n";
		help = help + "BdcsPowerOn:----------------------- " + "40" + "\r\n";
		help = help + "BdcsPowerOff:---------------------- " + "41" + "\r\n";
		help = help + "BdcsQueryPower:-------------------- " + "42" + "\r\n";
		help = help + "BdcCoast:-------------------------- " + "43 bdcIndex" + "\r\n";
		help = help + "BdcReverse:------------------------ " + "44 bdcIndex" + "\r\n";
		help = help + "BdcForward:------------------------ " + "45 bdcIndex" + "\r\n";
		help = help + "BdcBreak:-------------------------- " + "46 bdcIndex" + "\r\n";
		help = help + "BdcQuery:-------------------------- " + "47 bdcIndex" + "\r\n";
		help = help + "SteppersPowerOn:------------------- " + "60" + "\r\n";
		help = help + "SteppersPowerOff:------------------ " + "61" + "\r\n";
		help = help + "SteppersQueryPower:---------------- " + "62" + "\r\n";
		help = help + "StepperQueryResolution: ----------- " + "63" + "\r\n";
		help = help + "StepperConfigStep:----------------- " + "64 stepperIndex lowClks highClks" + "\r\n";
		help = help + "StepperAccelerationBuffer:--------- " + "65 stepperIndex value" + "\r\n";
		help = help + "StepperAccelerationBufferDecrement: " + "66 stepperIndex value" + "\r\n";
		help = help + "StepperDecelerationBuffer:          " + "67 stepperIndex value" + "\r\n";
		help = help + "StepperDecelerationBufferIncrement: " + "68 stepperIndex value" + "\r\n";
		help = help + "StepperEnable: -------------------- " + "69 stepperIndex 1/0" + "\r\n";
		help = help + "StepperForward: ------------------- " + "70 stepperIndex 1/0" + "\r\n";
		help = help + "StepperSteps: --------------------- " + "71 stepperIndex stepAmount" + "\r\n";
		help = help + "StepperRun: ----------------------- " + "72 stepperIndex intialPos finalPos" + "\r\n";
		help = help + "StepperConfigHome:----------------- " + "73 stepperIndex locatorIndex lineNumberStart lineNumberTerminal" + "\r\n";
		help = help + "StepperMove:----------------------- " + "74 stepperIndex forward stepAmount" + "\r\n";
		help = help + "StepperQuery: --------------------- " + "75 stepperIndex" + "\r\n";
		help = help + "StepperSetState: ------------------ " + "76 stepperIndex state" + "\r\n";
		help = help + "LocatorQuery:---------------------- " + "90 locatorIndex" + "\r\n";
		help = help + "BdcDelay:-------------------------- " + "200 ms" + "\r\n";
		help = help + "SaveMovementConfig:---------------- " + "300" + "\r\n";
		help = help + "SaveCoordinates:------------------- " + "350" + "\r\n";

		return help;
	}

	static std::string DevicesGet() 						{ return std::string("0\r\n"); }
	static std::string DeviceConnect(unsigned int index) 	{ return "1 " + std::to_string(index) + "\r\n"; }
	static std::string DeviceQueryPower() 					{ return std::string("2 \r\n"); }
	static std::string DeviceQueryFuse() 					{ return std::string("3 \r\n"); }
	static std::string OptPowerOn() 						{ return std::string("20 \r\n"); }
	static std::string OptPowerOff() 						{ return std::string("21 \r\n"); }
	static std::string OptQueryPower() 						{ return std::string("22 \r\n"); }
	static std::string DcmPowerOn() 						{ return std::string("30 \r\n"); } //todo: dcm index might be needed
	static std::string DcmPowerOff() 						{ return std::string("31 \r\n"); } //todo: dcm index might be needed
	static std::string DcmQueryPower() 						{ return std::string("32 \r\n"); }
	static std::string BdcsPowerOn() 						{ return std::string("40 \r\n"); }
	static std::string BdcsPowerOff() 						{ return std::string("41 \r\n"); }
	static std::string BdcsQueryPower() 					{ return std::string("42 \r\n"); }
	static std::string BdcCoast(unsigned int index) 		{ return "43 " + std::to_string(index) + "\r\n"; }
	static std::string BdcReverse(unsigned int index) 		{ return "44 " + std::to_string(index) + "\r\n"; }
	static std::string BdcForward(unsigned int index) 		{ return "45 " + std::to_string(index) + "\r\n"; }
	static std::string BdcBreak(unsigned int index) 		{ return "46 " + std::to_string(index) + "\r\n"; }
	static std::string BdcQuery(unsigned int index) 		{ return "47 " + std::to_string(index) + "\r\n"; }
	static std::string SteppersPowerOn() 					{ return std::string("60 \r\n"); }
	static std::string SteppersPowerOff() 					{ return std::string("61 \r\n"); }
	static std::string SteppersQueryPower() 				{ return std::string("62 \r\n"); }
	static std::string StepperQueryresolution() 			{ return std::string("63 \r\n"); }

	static std::string StepperConfigStep(
						unsigned int index,
						unsigned int lowClks,
						unsigned int highClks)
	{
		return "64 " + std::to_string(index) + " " + std::to_string(lowClks) + " " + std::to_string(highClks) + "\r\n";
	}

	static std::string StepperAccelerationBuffer(unsigned int index, unsigned long value)
	{
		return "65 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string StepperAccelerationBufferDecrement(unsigned int index, unsigned long value)
	{
		return "66 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string StepperDecelerationBuffer(unsigned int index, unsigned long value)
	{
		return "67 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string StepperDecelerationBufferIncrement(unsigned int index, unsigned long value)
	{
		return "68 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string StepperEnable(unsigned int index, bool enable)
	{
		if(enable)
			return "69 " + std::to_string(index) + " 1 \r\n";
		else
			return "69 " + std::to_string(index) + " 0 \r\n";
	}

	static std::string StepperForward(unsigned int index, bool forward)
	{
		if(forward)
			return "70 " + std::to_string(index) + " 1 \r\n";
		else
			return "70 " + std::to_string(index) + " 0 \r\n";
	}

	static std::string StepperSteps(unsigned int index, unsigned long value)
	{
		return "71 " + std::to_string(index) + " " + std::to_string(value) + "\r\n";
	}

	static std::string StepperRun(unsigned int index, unsigned long initialPos, unsigned long finalPos)
	{
		return "72 " + std::to_string(index) + " " + std::to_string(initialPos) + " " + std::to_string(finalPos) + "\r\n";
	}

	static std::string StepperConfigHome(unsigned int index,
										unsigned int locatorIndex,
										unsigned int locatorLineNumberStart,
										unsigned int locatorLineNumberTerminal)
	{
		return "73 " + std::to_string(index) + " " + std::to_string(locatorIndex) + " " + std::to_string(locatorLineNumberStart) + " " + std::to_string(locatorLineNumberTerminal) + "\r\n";
	}

	static std::string StepperQuery(unsigned int index) 		{ return "75 " + std::to_string(index) + "\r\n"; }
	static std::string LocatorQuery(unsigned int index) 		{ return "90 " + std::to_string(index) + "\r\n"; }
};




#endif /* CONSOLECOMMANDFACTORY_H_ */
