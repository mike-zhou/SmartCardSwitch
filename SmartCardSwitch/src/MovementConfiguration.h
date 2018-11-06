/*
 * MovementConfiguration.h
 *
 *  Created on: Nov 5, 2018
 *      Author: mikez
 */

#ifndef MOVEMENTCONFIGURATION_H_
#define MOVEMENTCONFIGURATION_H_

#include <string>
#include <vector>

class MovementConfiguration
{
public:
	MovementConfiguration(const std::string& pathFileName);
	bool PersistToFile();

	enum class Stepper
	{
		X = 0,
		Y,
		Z,
		W
	};

	bool SetStepperConfig(Stepper stepper,
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement,
						int locatorIndex,
						int locatorLineNumberStart,
						int locatorLineNumberTerminal);

	bool GetStepperConfig(Stepper stepper,
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement,
						int & locatorIndex,
						int & locatorLineNumberStart,
						int & locatorLineNumberTerminal);

	bool SetBdcDelay(long delay);
	bool GetBdcDelay(long & delay);

private:
	std::string _pathFileName;
	bool _configRestored;

	struct StepperMovementConfig
	{
		long lowClks;
		long highClks;
		long accelerationBuffer;
		long accelerationBufferDecrement;
		long decelerationBuffer;
		long decelerationBufferIncrement;
		int locatorIndex;
		int locatorLineNumberStart;
		int locatorLineNumberTerminal;

		StepperMovementConfig();
		std::string ToJsonObj();
	};

	std::vector<StepperMovementConfig> _steppers;

	long bdcDelay;
};



#endif /* MOVEMENTCONFIGURATION_H_ */
