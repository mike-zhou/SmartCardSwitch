/*
 * MovementConfiguration.cpp
 *
 *  Created on: Nov 6, 2018
 *      Author: mikez
 */

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

MovementConfiguration::MovementConfiguration(const std::string& pathFileName)
{

}

bool MovementConfiguration::PersistToFile()
{

}

bool MovementConfiguration::SetStepperConfig(Stepper stepper,
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

}


bool MovementConfiguration::GetStepperConfig(Stepper stepper,
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

}


bool MovementConfiguration::SetBdcDelay(long delay)
{

}

bool MovementConfiguration::GetBdcDelay(long & delay)
{

}

MovementConfiguration::StepperMovementConfig::StepperMovementConfig()
{

}

std::string MovementConfiguration::StepperMovementConfig::ToJsonObj()
{

}

