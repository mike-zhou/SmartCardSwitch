/*
 * ReplyTranslator.h
 *
 *  Created on: Oct 17, 2018
 *      Author: user1
 */

#ifndef REPLYTRANSLATOR_H_
#define REPLYTRANSLATOR_H_

#include <string>
#include <memory>
#include <vector>
#include "Poco/JSON/Object.h"

class ReplyTranslator
{
public:
	enum ReplyType
	{
		Invalid = -1,
		//replies to command
		DevicesGet,
		DeviceConnect,
		DeviceQueryPower,
		DeviceQueryFuse,
		OptPowerOn,
		OptPowerOff,
		OptQueryPower,
		DcmPowerOn,
		DcmPowerOff,
		DcmQueryPower,
		BdcsPowerOn,
		BdcsPowerOff,
		BdcsQueryPower,
		BdcCoast,
		BdcReverse,
		BdcForward,
		BdcBreak,
		BdcQuery,
		SteppersPowerOn,
		SteppersPowerOff,
		SteppersQueryPower,
		StepperQueryResolution,
		StepperConfigStep,
		StepperAccelerationBuffer,
		StepperAccelerationBufferDecrement,
		StepperDecelerationBuffer,
		StepperDecelerationBufferIncrement,
		StepperEnable,
		StepperForward,
		StepperSteps,
		StepperRun,
		StepperConfigHome,
		StepperMove,
		StepperQuery,
		StepperSetState,
		LocatorQuery,
		//events
		DevicePower = 10000,
		DeviceConnection,
		ProxyConnection,
		StepperOutOfRange
	};

	struct ReplyCommon
	{
		std::string originalString;
		std::string commandKey;
		unsigned short commandId; //proxy replies commandId in type of unsigned short.
		std::string errorInfo; //no error if empty
	};

	struct ReplyDevicesGet: ReplyCommon
	{
		std::vector<std::string> devices;
	};

	struct ReplyDeviceConnect: ReplyCommon
	{
		std::string deviceName;
		bool connected;
		std::string reason;
	};

	struct ReplyDeviceQueryPower: ReplyCommon
	{
		bool bPoweredOn;
	};

	struct ReplyDeviceQueryFuse: ReplyCommon
	{
		bool bFuseOn;
	};

	struct ReplyBdcsPowerOn: ReplyCommon
	{
		//empty
	};

	struct ReplyBdcsPowerOff: ReplyCommon
	{
		//empty
	};

	struct ReplyBdcsQueryPower: ReplyCommon
	{
		bool bPoweredOn;
	};

	struct ReplyBdcCoast: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyBdcReverse: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyBdcForward: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyBdcBreak: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyBdcQuery: ReplyCommon
	{
		enum BdcMode
		{
			COAST,
			REVERSE,
			FORWARD,
			BREAK
		} mode;
		unsigned int index;
	};

	struct ReplySteppersPowerOn: ReplyCommon
	{
		//empty
	};

	struct ReplySteppersPowerOff: ReplyCommon
	{
		//empty
	};

	struct ReplySteppersQueryPower: ReplyCommon
	{
		bool bPowered;
	};

	struct ReplyStepperQueryResolution: ReplyCommon
	{
		unsigned long resolutionUs;
	};

	struct ReplyStepperConfigStep: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperAccelerationBuffer: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperAccelerationBufferDecrement: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperDecelerationBuffer: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperDecelerationBufferIncrement: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperEnable: ReplyCommon
	{
		unsigned int index;
		bool enabled;
	};

	struct ReplyStepperForward: ReplyCommon
	{
		unsigned int index;
		bool forward;
	};

	struct ReplyStepperSteps: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperConfigHome: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyStepperRun: ReplyCommon
	{
		unsigned int index;
		unsigned long position;
	};

	struct ReplyStepperMove: ReplyCommon
	{
		unsigned int index;
		unsigned long position;
	};

	struct ReplyStepperQuery: ReplyCommon
	{
		unsigned int index;

		std::string state;
		bool bEnabled;
		bool bForward;
		unsigned int locatorIndex;
		unsigned int locatorLineNumberStart;
		unsigned int locatorLineNumberTerminal;
		unsigned long homeOffset;
		unsigned int lowClks;
		unsigned int highClks;
		unsigned int accelerationBuffer;
		unsigned int accelerationBufferDecrement;
		unsigned int decelerationBuffer;
		unsigned int decelerationBufferIncrement;
	};

	struct ReplyStepperSetState: ReplyCommon
	{
		unsigned int index;
	};

	struct ReplyLocatorQuery: ReplyCommon
	{
		unsigned int index;
		unsigned int lowInput;
	};

	struct EventDevicePower
	{
		std::string originalString;
		bool bPoweredOn;
	};

	struct EventDeviceConnection
	{
		std::string originalString;
		bool bConnected;
	};

	struct EventProxyConnection
	{
		std::string originalString;
		bool bConnected;
	};

	struct EventStepperOutOfRange
	{
		std::string originalString;
		int stepperIndex;
		int locatorNumber;
		int lineNumber;
	};



	ReplyTranslator(const std::string& reply);
	ReplyTranslator::ReplyType Type();

	std::shared_ptr<ReplyTranslator::ReplyDevicesGet> ToDevicesGet();
	std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> ToDeviceConnect();
	std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> ToDeviceQueryPower();
	std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> ToDeviceQueryFuse();
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> ToBdcsPowerOn();
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> ToBdcsPowerOff();
	std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> ToBdcsQueryPower();
	std::shared_ptr<ReplyTranslator::ReplyBdcCoast> ToBdcCoast();
	std::shared_ptr<ReplyTranslator::ReplyBdcReverse> ToBdcReverse();
	std::shared_ptr<ReplyTranslator::ReplyBdcForward> ToBdcForward();
	std::shared_ptr<ReplyTranslator::ReplyBdcBreak> ToBdcBreak();
	std::shared_ptr<ReplyTranslator::ReplyBdcQuery> ToBdcQuery();
	std::shared_ptr<ReplyTranslator::ReplySteppersPowerOn> ToSteppersPowerOn();
	std::shared_ptr<ReplyTranslator::ReplySteppersPowerOff> ToSteppersPowerOff();
	std::shared_ptr<ReplyTranslator::ReplySteppersQueryPower> ToSteppersQueryPower();
	std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> ToStepperQueryResolution();
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> ToStepperConfigStep();
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> ToStepperAccelerationBuffer();
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> ToStepperAccelerationBufferDecrement();
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> ToStepperDecelerationBuffer();
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> ToStepperDecelerationBufferIncrement();
	std::shared_ptr<ReplyTranslator::ReplyStepperEnable> ToStepperEnable();
	std::shared_ptr<ReplyTranslator::ReplyStepperForward> ToStepperForward();
	std::shared_ptr<ReplyTranslator::ReplyStepperSteps> ToStepperSteps();
	std::shared_ptr<ReplyTranslator::ReplyStepperRun> ToStepperRun();
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> ToStepperConfigHome();
	std::shared_ptr<ReplyTranslator::ReplyStepperMove> ToStepperMove();
	std::shared_ptr<ReplyTranslator::ReplyStepperQuery> ToStepperQuery();
	std::shared_ptr<ReplyTranslator::ReplyStepperSetState> ToStepperSetState();
	std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> ToLocatorQuery();
	//std::shared_ptr<ReplyTranslator::Reply> To();
	std::shared_ptr<ReplyTranslator::EventDevicePower> ToDevicePower();
	std::shared_ptr<ReplyTranslator::EventDeviceConnection> ToDeviceConnection();
	std::shared_ptr<ReplyTranslator::EventProxyConnection> ToProxyConnection();
	std::shared_ptr<ReplyTranslator::EventStepperOutOfRange> ToStepperOutOfRange();
	//std::shared_ptr<ReplyTranslator::Event> To();



private:
	std::string _reply;
	ReplyType _type;

	//replies to command
	const std::string strCommandDevicesGet = "devices get";
	const std::string strCommandDeviceConnect = "device connect";
	const std::string strCommandDeviceQueryPower = "device query power";
	const std::string strCommandDeviceQueryFuse = "device query fuse";
	const std::string strCommandBdcsPowerOn = "bdcs power on";
	const std::string strCommandBdcsPowerOff = "bdcs power off";
	const std::string strCommandBdcsQueryPower = "bdcs query power";
	const std::string strCommandBdcCoast = "bdc coast";
	const std::string strCommandBdcReverse = "bdc reverse";
	const std::string strCommandBdcForward = "bdc forward";
	const std::string strCommandBdcBreak = "bdc break";
	const std::string strCommandBdcQuery = "bdc query";
	const std::string strCommandSteppersPowerOn = "steppers power on";
	const std::string strCommandSteppersPowerOff = "steppers power off";
	const std::string strCommandSteppersQueryPower = "steppers query power";
	const std::string strCommandStepperQueryResolution = "stepper query resolution";
	const std::string strCommandStepperConfigStep = "stepper config step";
	const std::string strCommandStepperAccelerationBuffer = "stepper acceleration buffer";
	const std::string strCommandStepperAccelerationBufferDecrement = "stepper acceleration buffer decrement";
	const std::string strCommandStepperDecelerationBuffer = "stepper deceleration buffer";
	const std::string strCommandStepperDecelerationBufferIncrement = "stepper deceleration buffer increment";
	const std::string strCommandStepperEnable = "stepper enable";
	const std::string strCommandStepperForward = "stepper forward";
	const std::string strCommandStepperSteps = "stepper steps";
	const std::string strCommandStepperRun = "stepper run";
	const std::string strCommandStepperConfigHome = "stepper config home";
	const std::string strCommandStepperMove = "stepper move";
	const std::string strCommandStepperQuery = "stepper query";
	const std::string strCommandStepperSetState = "stepper set state";
	const std::string strCommandLocatorQuery = "locator query";
	//events
	const std::string strEventDevicePower = "device power";
	const std::string strEventDeviceConnection = "device connection";
	const std::string strEventProxyConnection = "proxy connection";
	const std::string strEventStepperOutOfRange = "stepper out of range";

	std::shared_ptr<ReplyTranslator::ReplyDevicesGet> _devicesGetPtr;
	std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> _deviceConnectPtr;
	std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> _deviceQueryPowerPtr;
	std::shared_ptr<ReplyTranslator::ReplyDeviceQueryFuse> _deviceQueryFusePtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> _bdcsPowerOnPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> _bdcsPowerOffPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> _bdcsQueryPowerPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcCoast> _bdcCoastPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcReverse> _bdcReversePtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcForward> _bdcForwardPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcBreak> _bdcBreakPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcQuery> _bdcQueryPtr;
	std::shared_ptr<ReplyTranslator::ReplySteppersPowerOn> _steppersPowerOnPtr;
	std::shared_ptr<ReplyTranslator::ReplySteppersPowerOff> _steppersPowerOffPtr;
	std::shared_ptr<ReplyTranslator::ReplySteppersQueryPower> _steppersQueryPowerPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> _stepperQueryResolutionPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> _stepperConfigStepPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> _stepperAccelerationBufferPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> _stepperAccelerationBufferDecrementPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> _stepperDecelerationBufferPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> _stepperDecelerationBufferIncrementPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperEnable> _stepperEnablePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperForward> _stepperForwardPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperSteps> _stepperStepsPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperRun> _stepperRunPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> _stepperConfigHomePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperMove> _stepperMovePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperQuery> _stepperQueryPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperSetState> _stepperSetStatePtr;
	std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> _locatorQueryPtr;
	std::shared_ptr<ReplyTranslator::EventDevicePower> _devicePowerPtr;
	std::shared_ptr<ReplyTranslator::EventDeviceConnection> _deviceConnectionPtr;
	std::shared_ptr<ReplyTranslator::EventProxyConnection> _proxyConnectionPtr;
	std::shared_ptr<ReplyTranslator::EventStepperOutOfRange> _stepperOutOfRangePtr;

	void parseReply(Poco::JSON::Object::Ptr objectPtr, const std::string& command);
	void parseEvent(Poco::JSON::Object::Ptr objectPtr, const std::string& event);
};



#endif /* REPLYTRANSLATOR_H_ */
