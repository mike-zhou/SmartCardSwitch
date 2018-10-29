/*
 * ConsoleOperator.h
 *
 *  Created on: Oct 18, 2018
 *      Author: user1
 */

#ifndef CONSOLEOPERATOR_H_
#define CONSOLEOPERATOR_H_

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

/***************************************
 * This class receives user input from a console,
 * creates JSON command accordingly, sends it to DeviceAccessor object,
 * and handles replies.
 ***************************************/
class ConsoleOperator: public Poco::Task, public IDeviceObserver
{
public:
	ConsoleOperator();

	void SetDevice(DeviceAccessor * pDeviceAccessor);

	void OnKeypressing(char key);

private:
	//IDeviceObserver
	virtual void OnFeedback(const std::string& feedback) override;
	//Poco::Task
	void runTask();

private:
	static const unsigned int BDC_AMOUNT = 6;

	Poco::Mutex _mutex;

	Poco::Event _event;
	std::deque<char> _input;
	std::deque<std::string> _feedbacks;
	DeviceAccessor * _pDeviceAccessor;

	void showHelp();

	void processInput();

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

	struct UserCommand
	{
		enum Type
		{
			Invalid = -1,
			DevicesGet = 0,
			DeviceConnect = 1,
			DeviceQueryPower = 2,
			DeviceQueryFuse = 3,
			OptPowerOn = 20,
			OptPowerOff = 21,
			OptQueryPower = 22,
			DcmPowerOn = 30,
			DcmPowerOff = 31,
			DcmQueryPower =32,
			BdcsPowerOn = 40,
			BdcsPowerOff = 41,
			BdcsQueryPower = 42,
			BdcCoast = 43,
			BdcReverse = 44,
			BdcForward = 45,
			BdcBreak = 46,
			BdcQuery = 47,
			SteppersPowerOn = 60,
			SteppersPowerOff = 61,
			SteppersQueryPower = 62,
			StepperQueryResolution = 63,
			StepperConfigStep = 64,
			StepperAccelerationBuffer = 65,
			StepperAccelerationBufferDecrement = 66,
			StepperDecelerationBuffer = 67,
			StepperDecelerationBufferIncrement = 68,
			StepperEnable = 69,
			StepperForward = 70,
			StepperSteps = 71,
			StepperRun = 72,
			StepperConfigHome = 73,
			StepperMove = 74,
			StepperQuery = 75,
			LocatorQuery = 90,
		} type;

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

		enum class BdcStatus
		{
			UNKNOWN,
			COAST,
			REVERSE,
			FORWARD,
			BREAK
		};

		CommandState state;
		std::string jsonCommandString;
		std::string commandKey;
		unsigned long commandId;
		std::string expectedResult;

		//command data
		int bdcIndex;
		int stepperIndex;
		int locatorIndex;

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
		BdcStatus resultBdcStatus[BDC_AMOUNT];

	} _userCommand;

	class Keyboard: public Poco::Runnable
	{
	public:
		Keyboard(ConsoleOperator * pReceiver);
		virtual ~Keyboard() {}

		void Terminate();

	private:
		virtual void run() override;

		bool _terminate;
		ConsoleOperator * _pReceiver;
	};

	Keyboard * _pKeyboardObj;
	Poco::Thread _keyboardThread;
};



#endif /* CONSOLEOPERATOR_H_ */
