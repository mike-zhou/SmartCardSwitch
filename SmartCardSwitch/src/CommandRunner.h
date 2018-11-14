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
 * This class receives user input from a console,
 * creates JSON command accordingly, sends it to DeviceAccessor object,
 * and handles replies.
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
	virtual CommandKey DevicesGet() override;
	virtual CommandKey DeviceConnect(unsigned int index) override;
	virtual CommandKey DeviceQueryPower() override;
	virtual CommandKey DeviceQueryFuse() override;
	virtual CommandKey OptPowerOn() override;
	virtual CommandKey OptPowerOff() override;
	virtual CommandKey OptQueryPower() override;
	virtual CommandKey DcmPowerOn() override;
	virtual CommandKey DcmPowerOff() override;
	virtual CommandKey DcmQueryPower() override;
	virtual CommandKey BdcsPowerOn() override;
	virtual CommandKey BdcsPowerOff() override;
	virtual CommandKey BdcsQueryPower() override;
	virtual CommandKey BdcCoast(unsigned int index) override;
	virtual CommandKey BdcReverse(unsigned int index) override;
	virtual CommandKey BdcForward(unsigned int index) override;
	virtual CommandKey BdcBreak(unsigned int index) override;
	virtual CommandKey BdcQuery(unsigned int index) override;
	virtual CommandKey SteppersPowerOn() override;
	virtual CommandKey SteppersPowerOff() override;
	virtual CommandKey SteppersQueryPower() override;
	virtual CommandKey StepperQueryResolution() override;
	virtual CommandKey StepperConfigStep(unsigned int index, unsigned short lowClks, unsigned short highClks) override;
	virtual CommandKey StepperAccelerationBuffer(unsigned int index, unsigned short value) override;
	virtual CommandKey StepperAccelerationBufferDecrement(unsigned int index, unsigned short value) override;
	virtual CommandKey StepperDecelerationBuffer(unsigned int index, unsigned short value) override;
	virtual CommandKey StepperDecelerationBufferIncrement(unsigned int index, unsigned short value) override;
	virtual CommandKey StepperEnable(unsigned int index, bool bEnable) override;
	virtual CommandKey StepperForward(unsigned int index, bool forward) override;
	virtual CommandKey StepperSteps(unsigned int index, unsigned short value) override;
	virtual CommandKey StepperRun(unsigned int index, unsigned short intialPos, unsigned short finalPos) override;
	virtual CommandKey StepperConfigHome(unsigned int index, unsigned int locatorIndex, unsigned int lineNumberStart, unsigned int lineNumberTerminal) override;
	virtual CommandKey StepperMove(unsigned int index, unsigned short steps) override;
	virtual CommandKey StepperQuery(unsigned int index) override;
	virtual CommandKey LocatorQuery(unsigned int index) override;
	virtual CommandKey BdcDelay(unsigned int index, unsigned int value) override;
	virtual CommandKey SaveMovementConfig() override;
	virtual CommandKey SaveCoordinates(unsigned int type, unsigned int index) override;

private:
	static const unsigned int BDC_AMOUNT = 6;
	static const unsigned int STEPPER_AMOUNT = 5;
	static const unsigned int LOCATOR_AMOUNT = 8;
	static const unsigned int LOCATOR_LINE_NUMBER_INVALID = 0;
	static const unsigned int LOCATOR_LINE_NUMBER_MIN = 1;
	static const unsigned int LOCATOR_LINE_NUMBER_MAX = 8;

	const std::string StepperStateApproachingHomeLocator = "approaching home locator";
	const std::string StepperStateLeavingHomeLocator = "Leaving Home";
	const std::string StepperStateGoingHome = "Going home";
	const std::string StepperStateKnownPosition = "Known position";
	const std::string StepperStateAccelerating = "Accelerating";
	const std::string StepperStateCruising = "Cruising";
	const std::string StepperStateDecelerating = "Decelerating";


	Poco::Mutex _mutex;
	Poco::Event _event;
	std::deque<std::string> _feedbacks;
	DeviceAccessor * _pDeviceAccessor;
	std::vector<IResponseReceiver *> _cmdResponseReceiverArray;

	unsigned long sendCmdToDevice(std::shared_ptr<DeviceCommand>& cmdPtr);
	void setBdcDelay(unsigned long delay);
	void saveMovementConfig();
	void saveCoordinates(unsigned int type, unsigned int index);
	void loadCoordinates();


	void processFeedbacks();
	bool isCorrespondingReply(const std::string& command, unsigned short commandId);
	void onFeedbackDevicesGet(std::shared_ptr<ReplyTranslator::ReplyDevicesGet> replyPtr);
	void onFeedbackDeviceConnect(std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> replyPtr);
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
	void onFeedbackLocatorQuery(std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> replyPtr);

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
		struct StepperStatus
		{
			std::string state;
			StepperEnableStatus enabled;
			StepperDirectionStatus forward;
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
		std::string expectedResult;

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
		//bdc delay
		unsigned long resultBdcDelay;

	} _userCommand;

};

#endif /* COMMANDRUNNER_H_ */
