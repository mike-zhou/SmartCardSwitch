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

	bool SetStepperBoundary(unsigned int index,
						int locatorIndex,
						int locatorLineNumberStart,
						int locatorLineNumberTerminal);

	bool SetStepperGeneral(unsigned int index,
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement);

	bool SetStepperForwardClockwise(unsigned int index, bool forwardClockwise);

	bool SetStepperCardInsert(
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement);

	bool SetStepperGoHome(
						long lowClks,
						long highClks,
						long accelerationBuffer,
						long accelerationBufferDecrement,
						long decelerationBuffer,
						long decelerationBufferIncrement);

	bool GetStepperBoundary(unsigned int index,
						int & locatorIndex,
						int & locatorLineNumberStart,
						int & locatorLineNumberTerminal);

	bool GetStepperGeneral(unsigned int index,
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement);

	bool GetStepperForwardClockwise(unsigned int index, bool & forwardClockwise);

	bool GetStepperCardInsert(
						long & lowClks,
						long & highClks,
						long & accelerationBuffer,
						long & accelerationBufferDecrement,
						long & decelerationBuffer,
						long & decelerationBufferIncrement);

	bool GetStepperGoHome(
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
