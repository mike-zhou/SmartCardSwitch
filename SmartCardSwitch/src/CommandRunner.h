/*
 * CommandRunner.h
 *
 *  Created on: Oct 16, 2018
 *      Author: user1
 */

#ifndef COMMANDRUNNER_H_
#define COMMANDRUNNER_H_

#include <memory>
#include <string>
#include <deque>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"
#include "DeviceAccessor.h"
#include "Command.h"
#include "CommandFactory.h"
#include "ReplyTranslator.h"
#include "ICommandReception.h"

/***************************************
 * This class implements functionalities in ICommandReception interface,
 * creates corresponding JSON command string,
 * passes it to DeviceAccessor object,
 * processes JSON reply from DeviceAccessor object
 ***************************************/
class CommandRunner: public Poco::Task, public ICommandReception, public IDeviceObserver
{
public:
	CommandRunner();

	void SetDevice(DeviceAccessor * pDeviceAccessor);
	virtual void AddResponseReceiver(IResponseReceiver * p) override;

private:
	//IDeviceObserver
	virtual void OnFeedback(const std::string& feedback) override;

	//Poco::Task
	void runTask();

	//ICommandReception
	virtual CommandId DevicesGet() override;
	virtual CommandId DeviceConnect(unsigned int index) override;
	virtual CommandId DeviceDelay(unsigned int clks) override;
	virtual CommandId DeviceQueryPower() override;
	virtual CommandId DeviceQueryFuse() override;
	virtual CommandId OptPowerOn() override;
	virtual CommandId OptPowerOff() override;
	virtual CommandId OptQueryPower() override;
	virtual CommandId DcmPowerOn(unsigned int index) override;
	virtual CommandId DcmPowerOff(unsigned int index) override;
	virtual CommandId DcmQueryPower(unsigned int index) override;
	virtual CommandId BdcsPowerOn() override;
	virtual CommandId BdcsPowerOff() override;
	virtual CommandId BdcsQueryPower() override;
	virtual CommandId BdcCoast(unsigned int index) override;
	virtual CommandId BdcReverse(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles) override;
	virtual CommandId BdcForward(unsigned int index, unsigned int lowClks, unsigned int highClks, unsigned int cycles) override;
	virtual CommandId BdcBreak(unsigned int index) override;
	virtual CommandId BdcQuery(unsigned int index) override;
	virtual CommandId SteppersPowerOn() override;
	virtual CommandId SteppersPowerOff() override;
	virtual CommandId SteppersQueryPower() override;
	virtual CommandId StepperQueryResolution() override;
	virtual CommandId StepperConfigStep(unsigned int index, unsigned short lowClks, unsigned short highClks) override;
	virtual CommandId StepperAccelerationBuffer(unsigned int index, unsigned short value) override;
	virtual CommandId StepperAccelerationBufferDecrement(unsigned int index, unsigned short value) override;
	virtual CommandId StepperDecelerationBuffer(unsigned int index, unsigned short value) override;
	virtual CommandId StepperDecelerationBufferIncrement(unsigned int index, unsigned short value) override;
	virtual CommandId StepperEnable(unsigned int index, bool bEnable) override;
	virtual CommandId StepperForward(unsigned int index, bool forward) override;
	virtual CommandId StepperSteps(unsigned int index, unsigned short value) override;
	virtual CommandId StepperRun(unsigned int index, unsigned short intialPos, unsigned short finalPos) override;
	virtual CommandId StepperConfigHome(unsigned int index, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal) override;
	virtual CommandId StepperMove(unsigned int index, unsigned short steps) override;
	virtual CommandId StepperQuery(unsigned int index) override;
	virtual CommandId StepperSetState(unsigned int index, StepperState state) override;
	virtual CommandId StepperForwardClockwise(unsigned int index, bool bForwardClockwise) override;
	virtual CommandId LocatorQuery(unsigned int index) override;
	virtual CommandId SaveMovementConfig() override;

private:
	static const unsigned int BDC_AMOUNT = 6;
	static const unsigned int STEPPER_AMOUNT = 5;
	static const unsigned int LOCATOR_AMOUNT = 8;
	static const unsigned int LOCATOR_LINE_NUMBER_INVALID = 0;
	static const unsigned int LOCATOR_LINE_NUMBER_MIN = 1;
	static const unsigned int LOCATOR_LINE_NUMBER_MAX = 8;
	static const unsigned int DCM_AMOUNT = 2;

	const std::string StepperStateApproachingHomeLocator = "approaching home locator";
	const std::string StepperStateLeavingHomeLocator = "leaving Home";
	const std::string StepperStateGoingHome = "going home";
	const std::string StepperStateKnownPosition = "known position";
	const std::string StepperStateAccelerating = "accelerating";
	const std::string StepperStateCruising = "cruising";
	const std::string StepperStateDecelerating = "decelerating";


	Poco::Mutex _mutex;
	Poco::Event _event;
	std::deque<std::string> _feedbacks;
	DeviceAccessor * _pDeviceAccessor;
	std::vector<IResponseReceiver *> _cmdResponseReceiverArray;

	unsigned long sendCmdToDevice(std::shared_ptr<DeviceCommand>& cmdPtr);
	void saveMovementConfig();

	void processFeedbacks();
	bool isCorrespondingReply(const std::string& command, unsigned short commandId);
	void onFeedbackDevicesGet(std::shared_ptr<ReplyTranslator::ReplyDevicesGet> replyPtr);
	void onFeedbackDeviceConnect(std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> replyPtr);
	void onFeedbackDeviceDelay(std::shared_ptr<ReplyTranslator::ReplyDeviceDelay> replyPtr);
	void onFeedbackDeviceQueryPower(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> replyPtr);
	void onFeedbackDeviceQueryFuse(std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> replyPtr);
	void onFeedbackBdcsPowerOn(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> replyPtr);
	void onFeedbackBdcsPowerOff(std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> replyPtr);
	void onFeedbackBdcsQueryPower(std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> replyPtr);
	void onFeedbackBdcCoast(std::shared_ptr<ReplyTranslator::ReplyBdcCoast> replyPtr);
	void onFeedbackBdcReverse(std::shared_ptr<ReplyTranslator::ReplyBdcReverse> replyPtr);
	void onFeedbackBdcForward(std::shared_ptr<ReplyTranslator::ReplyBdcForward> replyPtr);
	void onFeedbackBdcBreak(std::shared_ptr<ReplyTranslator::ReplyBdcBreak> replyPtr);
	void onFeedbackBdcQuery(std::shared_ptr<ReplyTranslator::ReplyBdcQuery> replyPtr);
	void onFeedbackSteppersPowerOn(std::shared_ptr<ReplyTranslator::ReplySteppersPowerOn> replyPtr);
	void onFeedbackSteppersPowerOff(std::shared_ptr<ReplyTranslator::ReplySteppersPowerOff> replyPtr);
	void onFeedbackSteppersQueryPower(std::shared_ptr<ReplyTranslator::ReplySteppersQueryPower> replyPtr);
	void onFeedbackStepperQueryResolution(std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> replyPtr);
	void onFeedbackStepperConfigStep(std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> replyPtr);
	void onFeedbackStepperAccelerationBuffer(std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> replyPtr);
	void onFeedbackStepperAccelerationBufferDecrement(std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> replyPtr);
	void onFeedbackStepperDecelerationBuffer(std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> replyPtr);
	void onFeedbackStepperDecelerationBufferIncrement(std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> replyPtr);
	void onFeedbackStepperEnable(std::shared_ptr<ReplyTranslator::ReplyStepperEnable> replyPtr);
	void onFeedbackStepperForward(std::shared_ptr<ReplyTranslator::ReplyStepperForward> replyPtr);
	void onFeedbackStepperSteps(std::shared_ptr<ReplyTranslator::ReplyStepperSteps> replyPtr);
	void onFeedbackStepperRun(std::shared_ptr<ReplyTranslator::ReplyStepperRun> replyPtr);
	void onFeedbackStepperConfigHome(std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> replyPtr);
	void onFeedbackStepperMove(std::shared_ptr<ReplyTranslator::ReplyStepperMove> replyPtr);
	void onFeedbackStepperQuery(std::shared_ptr<ReplyTranslator::ReplyStepperQuery> replyPtr);
	void onFeedbackStepperSetState(std::shared_ptr<ReplyTranslator::ReplyStepperSetState> replyPtr);
	void onFeedbackStepperForwardClockwise(std::shared_ptr<ReplyTranslator::ReplyStepperForwardClockwise> replyPtr);
	void onFeedbackLocatorQuery(std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> replyPtr);

	// "User" in the name is confusing because there is a "UserCommandRunner"
	// The following names of struct and variable need to be changed to avoid this confusion.
	struct UserCommand
	{
		enum class CommandState
		{
			IDLE = 0,
			COMMAND_SENT,
			CANCELLED,
			SUCCEEDED,
			FAILED
		};

		enum class PowerStatus
		{
			UNKNOWN,
			POWERED_ON,
			POWERED_OFF
		};

		enum class FuseStatus
		{
			UNKNOWN,
			FUSE_ON,
			FUSE_OFF
		};

		enum class StepperEnableStatus
		{
			UNKOWN,
			ENABLED,
			DISABLED
		};
		enum class StepperDirectionStatus
		{
			UNKNOWN,
			FORWORD,
			REVERSE
		};
		enum class StepperForwardClockwiseStatus
		{
			UNKNOWN,
			CLOCKWISE,
			COUNTER_CLOCKWISE
		};
		struct StepperStatus
		{
			std::string state;
			StepperEnableStatus enabled;
			StepperDirectionStatus forward;
			StepperForwardClockwiseStatus forwardClockwise;
			long locatorIndex;
			long locatorLineNumberStart;
			long locatorLineNumberTerminal;
			long homeOffset;
			long lowClks;
			long highClks;
			long accelerationBuffer;
			long accelerationBufferDecrement;
			long decelerationBuffer;
			long decelerationBufferIncrement;
		};

		//command type
		CommandState state;
		std::string jsonCommandString;
		std::string commandKey;
		unsigned long commandId;

		//command parameters
		int bdcIndex;
		int stepperIndex;
		int locatorIndex;
		int lowClks;
		int highClks;
		int accelerationBuffer;
		int accelerationBufferDecrement;
		int decelerationBuffer;
		int decelerationBufferIncrement;
		int steps;
		int initialPosition;
		int finalPosition;
		int locatorLineNumberStart;
		int locatorLineNumberTerminal;

		//results
		// DevicesGet
		std::vector<std::string> resultDevices;
		// DeviceConnect
		std::string resultConnectedDeviceName;
		//device query power
		PowerStatus resultDevicePowerStatus;
		//device query fuse
		FuseStatus resultDeviceFuseStatus;
		//BDCs power on
		//BDCs power off
		//BDCs query power
		PowerStatus resultBdcsPowerStatus;
		//BDC coast
		//BDC reverse
		//BDC forward
		//BDC break
		//BDC query
		IResponseReceiver::BdcStatus resultBdcStatus[BDC_AMOUNT];
		//steppers
		PowerStatus resultSteppersPowerStatus;
		long resultStepperClkResolution;
		StepperStatus resultStepperStatus[STEPPER_AMOUNT];
		//locator
		char resultLocatorStatus[LOCATOR_AMOUNT];

	} _userCommand;

};

#endif /* COMMANDRUNNER_H_ */
