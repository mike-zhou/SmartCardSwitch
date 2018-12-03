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

	bool SetStepperMovementCardInsert(
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement);

	bool SetStepperMovementGoHome(
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement);


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

	bool GetStepperMovementCardInsert(
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement);

	bool GetStepperMovementGoHome(
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement);


	void SetBdcConfig(unsigned long lowClks, unsigned long highClks, unsigned long cycles);
	void GetBdcConfig(unsigned long& lowClks, unsigned long& highClks, unsigned long& cycles);

private:
	const unsigned int STEPPERS_AMOUNT = 5;

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

	StepperMovementConfig _stepperMovementCardInsert;
	StepperMovementConfig _stepperMovementGoHome;

	struct BdcMovementConfig
	{
		long lowClks;
		long highClks;
		long cycles;

		BdcMovementConfig();
		std::string ToJsonObj();
	};
	BdcMovementConfig _bdc;
};



#endif /* MOVEMENTCONFIGURATION_H_ */
