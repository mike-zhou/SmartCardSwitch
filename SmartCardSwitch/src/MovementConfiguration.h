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

	bool SetStepperConfig(unsigned int index,
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement,
						int locatorIndex,
						int locatorLineNumberStart,
						int locatorLineNumberTerminal);

	bool GetStepperConfig(unsigned int index,
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement,
						int & locatorIndex,
						int & locatorLineNumberStart,
						int & locatorLineNumberTerminal);

	void SetBdcDelay(unsigned long delay) { _bdcDelay = delay; }
	unsigned long GetBdcDelay() { return _bdcDelay; }

private:
	const unsigned int STEPPERS_AMOUNT = 4;
	const unsigned int BDC_DELAY_DEFAULT = 1000;

	std::string _pathFileName;

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

	unsigned long _bdcDelay;
};



#endif /* MOVEMENTCONFIGURATION_H_ */
