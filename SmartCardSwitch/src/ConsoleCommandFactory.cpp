/*
 * ConsoleCommandFactory.cpp
 *
 *  Created on: Jan 2, 2019
 *      Author: user1
 */
#include <stdio.h>
#include "ConsoleCommandFactory.h"

std::string ConsoleCommandFactory::GetHelp()
{
	std::string help;

	help = help + "DevicesGet:------------------------ " + "0" + "\r\n";
	help = help + "DeviceConnect:--------------------- " + "1 deviceNumber" + "\r\n";
	help = help + "DeviceQueryPower:------------------ " + "2" + "\r\n";
	help = help + "DeviceQueryFuse:------------------- " + "3" + "\r\n";
	help = help + "DeviceDelay:----------------------- " + "4 clks" + "\r\n";
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
	help = help + "BdcReverse:------------------------ " + "44 bdcIndex lowClks highClks cycles" + "\r\n";
	help = help + "BdcForward:------------------------ " + "45 bdcIndex lowClks highClks cycles" + "\r\n";
	help = help + "BdcBreak:-------------------------- " + "46 bdcIndex" + "\r\n";
	help = help + "BdcQuery:-------------------------- " + "47 bdcIndex" + "\r\n";
	help = help + "SteppersPowerOn:------------------- " + "60" + "\r\n";
	help = help + "SteppersPowerOff:------------------ " + "61" + "\r\n";
	help = help + "SteppersQueryPower:---------------- " + "62" + "\r\n";
	help = help + "StepperQueryResolution: ----------- " + "63" + "\r\n";
	help = help + "StepperConfigStep:----------------- " + "64 stepperIndex lowClks highClks" + "\r\n";
	help = help + "StepperAccelerationBuffer:--------- " + "65 stepperIndex value" + "\r\n";
	help = help + "StepperAccelerationBufferDecrement: " + "66 stepperIndex value" + "\r\n";
	help = help + "StepperDecelerationBuffer:--------- " + "67 stepperIndex value" + "\r\n";
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
	help = help + "BdcConfig:------------------------- " + "200 lowClks highClks cycles" + "\r\n";
	help = help + "SaveMovementConfig:---------------- " + "300 type index" + "\r\n";
	help = help + "SaveMovementConfigStepperBoundary:- " + "301 stepperIndex" + "\r\n";
	help = help + "SaveMovementConfigStepperGeneral:-- " + "302 stepperIndex" + "\r\n";
	help = help + "SaveMovementConfigCardInsert:------ " + "303 stepperIndex" + "\r\n";
	help = help + "SaveMovementConfigGoHome:---------- " + "304 stepperIndex" + "\r\n";
	help = help + "SaveMovementConfigBdc:------------- " + "305 bdcIndex" + "\r\n";
	help = help + "SaveCoordinates:------------------- " + "350 type index" + "\r\n";
	help = help + "SaveSmartCardPlaceStartZ:---------- " + "351 value" + "\r\n";
	help = help + "SaveSmartCardFetchOffset:---------- " + "352 value" + "\r\n";
	help = help + "SaveSmartCardReleaseOffsetZ:------- " + "353 value" + "\r\n";
	help = help + "SaveSmartCardReaderSlowInsertEndY:- " + "354 value" + "\r\n";

	return help;
}

ConsoleCommandFactory::Type ConsoleCommandFactory::GetCmdType(const std::string& consoleCmd)
{
	Type rc;
	const unsigned int Amount = 10;
	int dataArray[Amount];

	for(unsigned int i=0; i<Amount; i++) {
		dataArray[i] = -1;
	}

	sscanf(consoleCmd.data(), "%d %d %d %d %d %d %d %d %d %d\r\n",
			dataArray,
			dataArray + 1,
			dataArray + 2,
			dataArray + 3,
			dataArray + 4,
			dataArray + 5,
			dataArray + 6,
			dataArray + 7,
			dataArray + 8,
			dataArray + 9);

	switch((Type)dataArray[0])
	{
		case Type::Invalid:
		case Type::DevicesGet:
		case Type::DeviceConnect:
		case Type::DeviceQueryPower:
		case Type::DeviceQueryFuse:
		case Type::DeviceDelay:
		case Type::OptPowerOn:
		case Type::OptPowerOff:
		case Type::OptQueryPower:
		case Type::DcmPowerOn:
		case Type::DcmPowerOff:
		case Type::DcmQueryPower:
		case Type::BdcsPowerOn:
		case Type::BdcsPowerOff:
		case Type::BdcsQueryPower:
		case Type::BdcCoast:
		case Type::BdcReverse:
		case Type::BdcForward:
		case Type::BdcBreak:
		case Type::BdcQuery:
		case Type::SteppersPowerOn:
		case Type::SteppersPowerOff:
		case Type::SteppersQueryPower:
		case Type::StepperQueryResolution:
		case Type::StepperConfigStep:
		case Type::StepperAccelerationBuffer:
		case Type::StepperAccelerationBufferDecrement:
		case Type::StepperDecelerationBuffer:
		case Type::StepperDecelerationBufferIncrement:
		case Type::StepperEnable:
		case Type::StepperForward:
		case Type::StepperSteps:
		case Type::StepperRun:
		case Type::StepperConfigHome:
		case Type::StepperMove:
		case Type::StepperQuery:
		case Type::StepperSetState:
		case Type::LocatorQuery:
		case Type::SaveMovementConfig:
		case Type::SaveMovementConfigStepperBoundary:
		case Type::SaveMovementConfigStepperGeneral:
		case Type::SaveMovementConfigCardInsert:
		case Type::SaveMovementConfigGoHome:
		case Type::SaveMovementConfigBdc:
		case Type::LoadMovementConfigStepper:
		case Type::SaveCoordinates:
		case Type::SaveCoordinateSmartCardPlaceStartZ:
		case Type::SaveCoordinateSmartCardFetchOffset:
		case Type::SaveCoordinateSmartCardReleaseOffsetZ:
		case Type::SaveCoordinateSmartCardReaderSlowInsertEndY:
			rc = (Type)dataArray[0];
			break;

		default:
			rc = Type::Invalid;
			break;
	}

	return rc;
}

bool ConsoleCommandFactory::GetParameterStepperIndex(const std::string & consoleCmd, unsigned int & stepperIndex)
{
	const unsigned int Amount = 10;
	int dataArray[Amount];

	for(unsigned int i=0; i<Amount; i++) {
		dataArray[i] = -1;
	}

	sscanf(consoleCmd.data(), "%d %d %d %d %d %d %d %d %d %d\r\n",
			dataArray,
			dataArray + 1,
			dataArray + 2,
			dataArray + 3,
			dataArray + 4,
			dataArray + 5,
			dataArray + 6,
			dataArray + 7,
			dataArray + 8,
			dataArray + 9);

	auto type = (Type)dataArray[0];

	switch(type)
	{
		case Type::StepperConfigStep:
		case Type::StepperAccelerationBuffer:
		case Type::StepperAccelerationBufferDecrement:
		case Type::StepperDecelerationBuffer:
		case Type::StepperDecelerationBufferIncrement:
		case Type::StepperEnable:
		case Type::StepperForward:
		case Type::StepperSteps:
		case Type::StepperRun:
		case Type::StepperConfigHome:
		case Type::StepperMove:
		case Type::StepperQuery:
		case Type::StepperSetState:
			stepperIndex = dataArray[1];
			return true;

		default:
			break;
	}

	return false;
}

bool ConsoleCommandFactory::GetParameterStepperSteps(const std::string & consoleCmd, unsigned int & steps)
{
	const unsigned int Amount = 10;
	int dataArray[Amount];

	for(unsigned int i=0; i<Amount; i++) {
		dataArray[i] = -1;
	}

	sscanf(consoleCmd.data(), "%d %d %d %d %d %d %d %d %d %d\r\n",
			dataArray,
			dataArray + 1,
			dataArray + 2,
			dataArray + 3,
			dataArray + 4,
			dataArray + 5,
			dataArray + 6,
			dataArray + 7,
			dataArray + 8,
			dataArray + 9);

	auto type = (Type)dataArray[0];

	switch(type)
	{
		case Type::StepperSteps:
			steps = dataArray[2];
			return true;

		default:
			break;
	}

	return false;
}

bool ConsoleCommandFactory::GetParameterStepperForward(const std::string & consoleCmd, bool & bForward)
{
	const unsigned int Amount = 10;
	int dataArray[Amount];

	for(unsigned int i=0; i<Amount; i++) {
		dataArray[i] = -1;
	}

	sscanf(consoleCmd.data(), "%d %d %d %d %d %d %d %d %d %d\r\n",
			dataArray,
			dataArray + 1,
			dataArray + 2,
			dataArray + 3,
			dataArray + 4,
			dataArray + 5,
			dataArray + 6,
			dataArray + 7,
			dataArray + 8,
			dataArray + 9);

	auto type = (Type)dataArray[0];

	switch(type)
	{
		case Type::StepperForward:
			bForward = (dataArray[2] != 0);
			return true;

		default:
			break;
	}

	return false;
}

bool ConsoleCommandFactory::GetParameterLocatorIndex(const std::string & consoleCmd, unsigned int & locatorIndex)
{
	const unsigned int Amount = 10;
	int dataArray[Amount];

	for(unsigned int i=0; i<Amount; i++) {
		dataArray[i] = -1;
	}

	sscanf(consoleCmd.data(), "%d %d %d %d %d %d %d %d %d %d\r\n",
			dataArray,
			dataArray + 1,
			dataArray + 2,
			dataArray + 3,
			dataArray + 4,
			dataArray + 5,
			dataArray + 6,
			dataArray + 7,
			dataArray + 8,
			dataArray + 9);

	auto type = (Type)dataArray[0];

	switch(type)
	{
		case Type::LocatorQuery:
			locatorIndex = dataArray[1];
			return true;

		default:
			break;
	}

	return false;
}
