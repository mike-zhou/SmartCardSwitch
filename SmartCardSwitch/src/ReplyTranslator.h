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
		//empty
	};

	struct ReplyBdcReverse: ReplyCommon
	{
		//empty
	};

	struct ReplyBdcForward: ReplyCommon
	{
		//empty
	};

	struct ReplyBdcBreak: ReplyCommon
	{
		//empty
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
	};

	struct ReplyStepperQueryResolution: ReplyCommon
	{
		unsigned long resolutionUs;
	};

	struct ReplyStepperConfigStep: ReplyCommon
	{
		unsigned long lowClks;
		unsigned long highClks;
	};

	struct ReplyStepperAccelerationBuffer: ReplyCommon
	{
		unsigned long buffer;
	};

	struct ReplyStepperAccelerationBufferDecrement: ReplyCommon
	{
		unsigned long decrement;
	};

	struct ReplyStepperDecelerationBuffer: ReplyCommon
	{
		unsigned long buffer;
	};

	struct ReplyStepperDecelerationBufferIncrement: ReplyCommon
	{
		unsigned long increment;;
	};

	struct ReplyStepperEnable: ReplyCommon
	{
		bool bEnabled;
	};

	struct ReplyStepperConfigHome: ReplyCommon
	{
		unsigned long position;
	};

	struct ReplyStepperMove: ReplyCommon
	{
		unsigned long position;
	};

	struct ReplyStepperQuery: ReplyCommon
	{
		struct StepperStatus
		{
			bool bEnabled;
			bool bForward;
			unsigned long position;
		};

		std::vector<StepperStatus> status;
	};

	struct ReplyLocatorQuery: ReplyCommon
	{
		std::vector<std::vector<int>> status;
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
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> ToBdcsPowerOn();
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> ToBdcsPowerOff();
	std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> ToBdcsQueryPower();
	std::shared_ptr<ReplyTranslator::ReplyBdcCoast> ToBdcCoast();
	std::shared_ptr<ReplyTranslator::ReplyBdcReverse> ToBdcReverse();
	std::shared_ptr<ReplyTranslator::ReplyBdcForward> ToBdcForward();
	std::shared_ptr<ReplyTranslator::ReplyBdcBreak> ToBdcBreak();
	std::shared_ptr<ReplyTranslator::ReplyBdcQuery> ToBdcQuery();
	std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> ToStepperQueryResolution();
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> ToStepperConfigStep();
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> ToStepperAccelerationBuffer();
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> ToStepperAccelerationBufferDecrement();
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> ToStepperDecelerationBuffer();
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> ToStepperDecelerationBufferIncrement();
	std::shared_ptr<ReplyTranslator::ReplyStepperEnable> ToStepperEnable();
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> ToStepperConfigHome();
	std::shared_ptr<ReplyTranslator::ReplyStepperMove> ToStepperMove();
	std::shared_ptr<ReplyTranslator::ReplyStepperQuery> ToStepperQuery();
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
	const std::string strCommandLocatorQuery = "locator query";
	//events
	const std::string strEventDevicePower = "device power";
	const std::string strEventDeviceConnection = "device connection";
	const std::string strEventProxyConnection = "proxy connection";
	const std::string strEventStepperOutOfRange = "stepper out of range";

	std::shared_ptr<ReplyTranslator::ReplyDevicesGet> _devicesGetPtr;
	std::shared_ptr<ReplyTranslator::ReplyDeviceConnect> _deviceConnectPtr;
	std::shared_ptr<ReplyTranslator::ReplyDeviceQueryPower> _deviceQueryPowerPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOn> _bdcsPowerOnPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsPowerOff> _bdcsPowerOffPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcsQueryPower> _bdcsQueryPowerPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcCoast> _bdcCoastPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcReverse> _bdcReversePtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcForward> _bdcForwardPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcBreak> _bdcBreakPtr;
	std::shared_ptr<ReplyTranslator::ReplyBdcQuery> _bdcQueryPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperQueryResolution> _stepperQueryResolutionPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigStep> _stepperConfigStepPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBuffer> _stepperAccelerationBufferPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperAccelerationBufferDecrement> _stepperAccelerationBufferDecrementPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBuffer> _stepperDecelerationBufferPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperDecelerationBufferIncrement> _stepperDecelerationBufferIncrementPtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperEnable> _stepperEnablePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperConfigHome> _stepperConfigHomePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperMove> _stepperMovePtr;
	std::shared_ptr<ReplyTranslator::ReplyStepperQuery> _stepperQueryPtr;
	std::shared_ptr<ReplyTranslator::ReplyLocatorQuery> _locatorQueryPtr;
	std::shared_ptr<ReplyTranslator::EventDevicePower> _devicePowerPtr;
	std::shared_ptr<ReplyTranslator::EventDeviceConnection> _deviceConnectionPtr;
	std::shared_ptr<ReplyTranslator::EventProxyConnection> _proxyConnectionPtr;
	std::shared_ptr<ReplyTranslator::EventStepperOutOfRange> _stepperOutOfRangePtr;

	void parseReply(Poco::JSON::Object::Ptr objectPtr, const std::string& command);
	void parseEvent(Poco::JSON::Object::Ptr objectPtr, const std::string& event);
};



#endif /* REPLYTRANSLATOR_H_ */
