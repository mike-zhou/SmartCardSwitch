/*
 * MovementConfiguration.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: mikez
 */

#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Exception.h"
#include "Poco/Format.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Logger.h"

#include "MovementConfiguration.h"

extern Logger * pLogger;

MovementConfiguration::MovementConfiguration(const std::string& pathFileName)
{
	_pathFileName = pathFileName;

	if(_pathFileName.empty()) {
		pLogger->LogError("MovementConfiguration::MovementConfiguration empty file path & name");
		return;
	}

	Poco::File storageFile(_pathFileName);
	if(storageFile.exists() == false) {
		pLogger->LogError("MovementConfiguration::MovementConfiguration file not exist: " + _pathFileName);
		return;
	}

	try
	{
		std::string json;

		//open storage file
		int fd = open(_pathFileName.c_str(), O_RDONLY);
		if(fd < 0) {
			pLogger->LogError("MovementConfiguration::MovementConfiguration cannot open file: " + _pathFileName);
			return;
		}
		//read out the file content
		for(;;)
		{
			unsigned char c;
			auto amount = read(fd, &c, 1);
			if(amount < 1) {
				break;
			}
			else {
				json.push_back(c);
			}
		}
		//close file
		close(fd);

		if(json.empty()) {
			pLogger->LogError("MovementConfiguration::MovementConfiguration nothing read from: " + _pathFileName);
		}
		else
		{
			//parse file content
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(json);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			//restore data for steppers
			auto steppersAmount = ds["steppers"].size();
			for(unsigned int i=0; i<steppersAmount; i++)
			{
				unsigned int index;

				bool forwardClockwise;
				long lowClks;
				long highClks;
				long accelerationBuffer;
				long accelerationBufferDecrement;
				long decelerationBuffer;
				long decelerationBufferIncrement;
				int locatorIndex;
				int locatorLineNumberStart;
				int locatorLineNumberTerminal;

				index 						= ds["steppers"][i]["index"];
				forwardClockwise			= ds["steppers"][i]["value"]["forwardClockwise"];
				lowClks 					= ds["steppers"][i]["value"]["lowClks"];
				highClks 					= ds["steppers"][i]["value"]["highClks"];
				accelerationBuffer 			= ds["steppers"][i]["value"]["accelerationBuffer"];
				accelerationBufferDecrement = ds["steppers"][i]["value"]["accelerationBufferDecrement"];
				decelerationBuffer 			= ds["steppers"][i]["value"]["decelerationBuffer"];
				decelerationBufferIncrement = ds["steppers"][i]["value"]["decelerationBufferIncrement"];
				locatorIndex 				= ds["steppers"][i]["value"]["locatorIndex"];
				locatorLineNumberStart 		= ds["steppers"][i]["value"]["locatorLineNumberStart"];
				locatorLineNumberTerminal 	= ds["steppers"][i]["value"]["locatorLineNumberTerminal"];

				SetStepperGeneral(index,
									lowClks,
									highClks,
									accelerationBuffer,
									accelerationBufferDecrement,
									decelerationBuffer,
									decelerationBufferIncrement);

				SetStepperBoundary(index,
									locatorIndex,
									locatorLineNumberStart,
									locatorLineNumberTerminal);

				SetStepperForwardClockwise(index, forwardClockwise);
			}

			//cardInsert
			_stepperMovementCardInsert.lowClks = 						ds["cardInsert"]["lowClks"];
			_stepperMovementCardInsert.highClks = 						ds["cardInsert"]["highClks"];
			_stepperMovementCardInsert.accelerationBuffer = 			ds["cardInsert"]["accelerationBuffer"];
			_stepperMovementCardInsert.accelerationBufferDecrement = 	ds["cardInsert"]["accelerationBufferDecrement"];
			_stepperMovementCardInsert.decelerationBuffer = 			ds["cardInsert"]["decelerationBuffer"];
			_stepperMovementCardInsert.decelerationBufferIncrement = 	ds["cardInsert"]["decelerationBufferIncrement"];

			//goHome
			_stepperMovementGoHome.lowClks = 						ds["goHome"]["lowClks"];
			_stepperMovementGoHome.highClks = 						ds["goHome"]["highClks"];
			_stepperMovementGoHome.accelerationBuffer = 			ds["goHome"]["accelerationBuffer"];
			_stepperMovementGoHome.accelerationBufferDecrement = 	ds["goHome"]["accelerationBufferDecrement"];
			_stepperMovementGoHome.decelerationBuffer = 			ds["goHome"]["decelerationBuffer"];
			_stepperMovementGoHome.decelerationBufferIncrement = 	ds["goHome"]["decelerationBufferIncrement"];

			//restore data for BDC
			_bdc.lowClks = ds["bdc"]["lowClks"];
			_bdc.highClks = ds["bdc"]["highClks"];
			_bdc.cycles = ds["bdc"]["cycles"];

			pLogger->LogInfo("MovementConfiguration::MovementConfiguration storage file is parsed successfully");
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("MovementConfiguration::MovementConfiguration exception: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("MovementConfiguration::MovementConfiguration unknown exception");
	}
}

bool MovementConfiguration::PersistToFile()
{
	std::string json;
	bool rc = false;

	//create the json string
	json = "{";

	//steppers
	json = json + "\"steppers\": [";
	for(unsigned int i=0; i<_steppers.size(); i++)
	{
		json = json + "{\"index\":" + std::to_string(i) + ",\"value\":" + _steppers[i].ToJsonObj() + "},";
	}
	if(!_steppers.empty()) {
		json.pop_back();//delete the extra ','
	}
	json = json + "]";//end of steppers

	//cardInsert
	json = json + ",\"cardInsert\":" + _stepperMovementCardInsert.ToJsonObj();

	//goHome
	json = json + ",\"goHome\":" + _stepperMovementGoHome.ToJsonObj();

	//BDC delay
	json = json + ",\"bdc\":" + _bdc.ToJsonObj();

	json = json + "}";

	//write json string to file
	try
	{
		int fd;
		Poco::File storageFile(_pathFileName);

		if(storageFile.exists()) {
			storageFile.remove(false);
		}

		fd = open(_pathFileName.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if(fd >= 0)
		{
			pLogger->LogInfo("MovementConfiguration::PersistToFile write " + std::to_string(json.size()) + " bytes to file " + _pathFileName);
			auto amount = write(fd, json.c_str(), json.size());
			if(amount != json.size()) {
				pLogger->LogError("MovementConfiguration::PersistToFile failure in writing: " + std::to_string(amount) + "/" + std::to_string(json.size()));
			}
			else {
				rc = true;
			}
			close(fd);
		}
		else
		{
			pLogger->LogError("MovementConfiguration::PersistToFile cannot open " + _pathFileName);
		}
	}
	catch(Poco::Exception& e)
	{
		pLogger->LogError("MovementConfiguration::PersistToFile exception: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("MovementConfiguration::PersistToFile unknown exception");
	}

	return rc;
}

bool MovementConfiguration::SetStepperBoundary(unsigned int index,
					int locatorIndex,
					int locatorLineNumberStart,
					int locatorLineNumberTerminal)
{
	bool rc = false;
	char buf[256];

	sprintf(buf, "MovementConfiguration::SetStepperBoundary index: %d, locatorIndex: %d, start: %d, terminal: %d", index, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
	std::string info(buf);
	pLogger->LogInfo(info);

	if(index < STEPPERS_AMOUNT)
	{
		if(index >= _steppers.size())
		{
			for(; _steppers.size() < STEPPERS_AMOUNT; ) {
				StepperMovementConfig defaultCfg;
				_steppers.push_back(defaultCfg);
			}
		}

		auto& stepper = _steppers[index];
		stepper.locatorIndex = locatorIndex;
		stepper.locatorLineNumberStart = locatorLineNumberStart;
		stepper.locatorLineNumberTerminal = locatorLineNumberTerminal;

		rc = true;
	}

	return rc;
}


bool MovementConfiguration::SetStepperGeneral(unsigned int index,
					long lowClks,
					long highClks,
					long accelerationBuffer,
					long accelerationBufferDecrement,
					long decelerationBuffer,
					long decelerationBufferIncrement)
{
	bool rc = false;
	char buf[512];

	sprintf(buf, "MovementConfiguration::SetStepperGeneral index: %d, lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			index, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	if(index < STEPPERS_AMOUNT)
	{
		if(index >= _steppers.size())
		{
			for(; _steppers.size() < STEPPERS_AMOUNT; ) {
				StepperMovementConfig defaultCfg;
				_steppers.push_back(defaultCfg);
			}
		}

		auto& stepper = _steppers[index];
		stepper.lowClks = lowClks;
		stepper.highClks = highClks;
		stepper.accelerationBuffer = accelerationBuffer;
		stepper.accelerationBufferDecrement = accelerationBufferDecrement;
		stepper.decelerationBuffer = decelerationBuffer;
		stepper.decelerationBufferIncrement = decelerationBufferIncrement;

		rc = true;
	}

	return rc;
}

bool MovementConfiguration::SetStepperForwardClockwise(unsigned int index, bool forwardClockwise)
{
	bool rc = false;
	char buf[512];

	sprintf(buf, "MovementConfiguration::SetStepperForwardClockwise index: %d, clockwise: %s",
			index, forwardClockwise?"true":"false");
	std::string info(buf);
	pLogger->LogInfo(info);

	if(index < STEPPERS_AMOUNT)
	{
		if(index >= _steppers.size())
		{
			for(; _steppers.size() < STEPPERS_AMOUNT; ) {
				StepperMovementConfig defaultCfg;
				_steppers.push_back(defaultCfg);
			}
		}

		auto& stepper = _steppers[index];
		stepper.forwardClockwise = forwardClockwise;

		rc = true;
	}

	return rc;
}

bool MovementConfiguration::SetStepperCardInsert(
					long lowClks,
					long highClks,
					long accelerationBuffer,
					long accelerationBufferDecrement,
					long decelerationBuffer,
					long decelerationBufferIncrement)
{
	char buf[512];

	sprintf(buf, "MovementConfiguration::SetStepperCardInsert lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	_stepperMovementCardInsert.lowClks = lowClks;
	_stepperMovementCardInsert.highClks = highClks;
	_stepperMovementCardInsert.accelerationBuffer = accelerationBuffer;
	_stepperMovementCardInsert.accelerationBufferDecrement = accelerationBufferDecrement;
	_stepperMovementCardInsert.decelerationBuffer = decelerationBuffer;
	_stepperMovementCardInsert.decelerationBufferIncrement = decelerationBufferIncrement;

	return true;
}

bool MovementConfiguration::SetStepperGoHome(
					long lowClks,
					long highClks,
					long accelerationBuffer,
					long accelerationBufferDecrement,
					long decelerationBuffer,
					long decelerationBufferIncrement)
{
	char buf[512];

	sprintf(buf, "MovementConfiguration::SetStepperGoHome lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	_stepperMovementGoHome.lowClks = lowClks;
	_stepperMovementGoHome.highClks = highClks;
	_stepperMovementGoHome.accelerationBuffer = accelerationBuffer;
	_stepperMovementGoHome.accelerationBufferDecrement = accelerationBufferDecrement;
	_stepperMovementGoHome.decelerationBuffer = decelerationBuffer;
	_stepperMovementGoHome.decelerationBufferIncrement = decelerationBufferIncrement;

	return true;
}

bool MovementConfiguration::GetStepperBoundary(unsigned int index,
					int & locatorIndex,
					int & locatorLineNumberStart,
					int & locatorLineNumberTerminal)
{
	if(index >= STEPPERS_AMOUNT) {
		return false;
	}
	if(index >= _steppers.size()) {
		return false;
	}

	auto& stepper = _steppers[index];
	locatorIndex = stepper.locatorIndex;
	locatorLineNumberStart = stepper.locatorLineNumberStart;
	locatorLineNumberTerminal = stepper.locatorLineNumberTerminal;

	char buf[256];

	sprintf(buf, "MovementConfiguration::GetStepperBoundary index: %d, locatorIndex: %d, start: %d, terminal: %d", index, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
	std::string info(buf);
	pLogger->LogInfo(info);

	return true;
}

bool MovementConfiguration::GetStepperGeneral(unsigned int index,
					long & lowClks,
					long & highClks,
					long & accelerationBuffer,
					long & accelerationBufferDecrement,
					long & decelerationBuffer,
					long & decelerationBufferIncrement)
{
	if(index >= STEPPERS_AMOUNT) {
		return false;
	}
	if(index >= _steppers.size()) {
		return false;
	}

	auto& stepper = _steppers[index];
	lowClks = stepper.lowClks;
	highClks = stepper.highClks;
	accelerationBuffer = stepper.accelerationBuffer;
	accelerationBufferDecrement = stepper.accelerationBufferDecrement;
	decelerationBuffer = stepper.decelerationBuffer;
	decelerationBufferIncrement = stepper.decelerationBufferIncrement;

	char buf[512];

	sprintf(buf, "MovementConfiguration::GetStepperGeneral index: %d, lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			index, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	return true;
}

bool MovementConfiguration::GetStepperForwardClockwise(unsigned int index, bool & forwardClockwise)
{
	if(index >= STEPPERS_AMOUNT) {
		return false;
	}
	if(index >= _steppers.size()) {
		return false;
	}

	forwardClockwise = _steppers[index].forwardClockwise;


	char buf[512];

	sprintf(buf, "MovementConfiguration::GetStepperForwardClockwise index: %d, forward clockwise: %s",
			index, forwardClockwise?"true":"false");
	std::string info(buf);
	pLogger->LogInfo(info);

	return true;
}

bool MovementConfiguration::GetStepperCardInsert(
					long & lowClks,
					long & highClks,
					long & accelerationBuffer,
					long & accelerationBufferDecrement,
					long & decelerationBuffer,
					long & decelerationBufferIncrement)
{
	lowClks = _stepperMovementCardInsert.lowClks;
	highClks = _stepperMovementCardInsert.highClks;
	accelerationBuffer = _stepperMovementCardInsert.accelerationBuffer;
	accelerationBufferDecrement = _stepperMovementCardInsert.accelerationBufferDecrement;
	decelerationBuffer = _stepperMovementCardInsert.decelerationBuffer;
	decelerationBufferIncrement = _stepperMovementCardInsert.decelerationBufferIncrement;

	char buf[512];

	sprintf(buf, "MovementConfiguration::GetStepperCardInsert lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	return true;
}

bool MovementConfiguration::GetStepperGoHome(
					long & lowClks,
					long & highClks,
					long & accelerationBuffer,
					long & accelerationBufferDecrement,
					long & decelerationBuffer,
					long & decelerationBufferIncrement)
{
	lowClks = _stepperMovementGoHome.lowClks;
	highClks = _stepperMovementGoHome.highClks;
	accelerationBuffer = _stepperMovementGoHome.accelerationBuffer;
	accelerationBufferDecrement = _stepperMovementGoHome.accelerationBufferDecrement;
	decelerationBuffer = _stepperMovementGoHome.decelerationBuffer;
	decelerationBufferIncrement = _stepperMovementGoHome.decelerationBufferIncrement;

	char buf[512];

	sprintf(buf, "MovementConfiguration::GetStepperGoHome lowClks: %ld, highClks: %ld, accelerationBuffer: %ld, accelerationBufferDecrement: %ld, decelerationBuffer: %ld, decelerationBufferIncrement: %ld",
			lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement);
	std::string info(buf);
	pLogger->LogInfo(info);

	return true;
}


void MovementConfiguration::SetBdcConfig(unsigned long lowClks, unsigned long highClks, unsigned long cycles)
{
	char buf[512];

	sprintf(buf, "MovementConfiguration::SetBdcConfig lowClks: %ld, highClks: %ld, cycles: %ld",
			lowClks, highClks, cycles);
	std::string info(buf);
	pLogger->LogInfo(info);

	_bdc.lowClks = lowClks;
	_bdc.highClks = highClks;
	_bdc.cycles = cycles;
}

void MovementConfiguration::GetBdcConfig(unsigned long& lowClks, unsigned long& highClks, unsigned long& cycles)
{
	lowClks = _bdc.lowClks;
	highClks = _bdc.highClks;
	cycles = _bdc.cycles;

	char buf[512];

	sprintf(buf, "MovementConfiguration::GetBdcConfig lowClks: %ld, highClks: %ld, cycles: %ld",
			lowClks, highClks, cycles);
	std::string info(buf);
	pLogger->LogInfo(info);
}


MovementConfiguration::StepperMovementConfig::StepperMovementConfig()
{
	forwardClockwise = true;
	lowClks = 0;
	highClks = 0;
	accelerationBuffer = 0;
	accelerationBufferDecrement = 0;
	decelerationBuffer = 0;
	decelerationBufferIncrement = 0;
	locatorIndex = 0;
	locatorLineNumberStart = 0;
	locatorLineNumberTerminal = 0;
}

std::string MovementConfiguration::StepperMovementConfig::ToJsonObj()
{
	std::string json;

	json = "{";
	json = json + "\"forwardClockwise\":" + std::string(forwardClockwise ? "true" : "false");
	json = json + ",\"lowClks\":" + std::to_string(lowClks);
	json = json + ",\"highClks\":" + std::to_string(highClks);
	json = json + ",\"accelerationBuffer\":" + std::to_string(accelerationBuffer);
	json = json + ",\"accelerationBufferDecrement\":" + std::to_string(accelerationBufferDecrement);
	json = json + ",\"decelerationBuffer\":" + std::to_string(decelerationBuffer);
	json = json + ",\"decelerationBufferIncrement\":" + std::to_string(decelerationBufferIncrement);
	json = json + ",\"locatorIndex\":" + std::to_string(locatorIndex);
	json = json + ",\"locatorLineNumberStart\":" + std::to_string(locatorLineNumberStart);
	json = json + ",\"locatorLineNumberTerminal\":" + std::to_string(locatorLineNumberTerminal);
	json += "}";

	return json;
}

MovementConfiguration::BdcMovementConfig::BdcMovementConfig()
{
	lowClks = 1;
	highClks = 1;
	cycles = 1;
}

std::string MovementConfiguration::BdcMovementConfig::ToJsonObj()
{
	std::string json;

	json = "{";
	json = json + "\"lowClks\":" + std::to_string(lowClks);
	json = json + ",\"highClks\":" + std::to_string(highClks);
	json = json + ",\"cycles\":" + std::to_string(cycles);
	json += "}";

	return json;
}

