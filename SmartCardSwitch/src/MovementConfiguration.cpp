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
	_bdcDelay = BDC_DELAY_DEFAULT;

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
			pLogger->LogError("MovementConfiguration::PersistToFile cannot open file: " + _pathFileName);
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
			pLogger->LogError("MovementConfiguration::PersistToFile nothing read from: " + _pathFileName);
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
				lowClks 					= ds["steppers"][i]["value"]["lowClks"];
				highClks 					= ds["steppers"][i]["value"]["highClks"];
				accelerationBuffer 			= ds["steppers"][i]["value"]["accelerationBuffer"];
				accelerationBufferDecrement = ds["steppers"][i]["value"]["accelerationBufferDecrement"];
				decelerationBuffer 			= ds["steppers"][i]["value"]["decelerationBuffer"];
				decelerationBufferIncrement = ds["steppers"][i]["value"]["decelerationBufferIncrement"];
				locatorIndex 				= ds["steppers"][i]["value"]["locatorIndex"];
				locatorLineNumberStart 		= ds["steppers"][i]["value"]["locatorLineNumberStart"];
				locatorLineNumberTerminal 	= ds["steppers"][i]["value"]["locatorLineNumberTerminal"];

				SetStepperConfig(index,
									lowClks,
									highClks,
									accelerationBuffer,
									accelerationBufferDecrement,
									decelerationBuffer,
									decelerationBufferIncrement,
									locatorIndex,
									locatorLineNumberStart,
									locatorLineNumberTerminal);
			}

			//restore data for BDC
			_bdcDelay = ds["bdcDelay"];

			pLogger->LogInfo("MovementConfiguration::PersistToFile storage file is parsed successfully");
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

	//BDC delay
	json = json + ",\"bdcDelay\":" + std::to_string(_bdcDelay);

	json = json + "}";

	//write json string to file
	try
	{
		int fd;
		Poco::File storageFile(_pathFileName);

		if(storageFile.exists()) {
			storageFile.remove(false);
		}

		fd = open(_pathFileName.c_str(), O_CREAT | O_WRONLY);
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

bool MovementConfiguration::SetStepperConfig(unsigned int index,
					long lowClks,
					long highClks,
					long accelerationBuffer,
					long accelerationBufferDecrement,
					long decelerationBuffer,
					long decelerationBufferIncrement,
					int locatorIndex,
					int locatorLineNumberStart,
					int locatorLineNumberTerminal)
{
	bool rc = false;

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
		stepper.locatorIndex = locatorIndex;
		stepper.locatorLineNumberStart = locatorLineNumberStart;
		stepper.locatorLineNumberTerminal = locatorLineNumberTerminal;

		rc = true;
	}

	return rc;
}


bool MovementConfiguration::GetStepperConfig(unsigned int index,
					long & lowClks,
					long & highClks,
					long & accelerationBuffer,
					long & accelerationBufferDecrement,
					long & decelerationBuffer,
					long & decelerationBufferIncrement,
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
	lowClks = stepper.lowClks;
	highClks = stepper.highClks;
	accelerationBuffer = stepper.accelerationBuffer;
	accelerationBufferDecrement = stepper.accelerationBufferDecrement;
	decelerationBuffer = stepper.decelerationBuffer;
	decelerationBufferIncrement = stepper.decelerationBufferIncrement;
	locatorIndex = stepper.locatorIndex;
	locatorLineNumberStart = stepper.locatorLineNumberStart;
	locatorLineNumberTerminal = stepper.locatorLineNumberTerminal;

	return true;
}


MovementConfiguration::StepperMovementConfig::StepperMovementConfig()
{
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
	json = json + "\"lowClks\":" + std::to_string(lowClks);
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


