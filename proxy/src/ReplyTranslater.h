/*
 * ReplyTranslater.h
 *
 *  Created on: Oct 26, 2018
 *      Author: mikez
 */

#ifndef REPLYTRANSLATER_H_
#define REPLYTRANSLATER_H_

#include <string>
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"

//translate device reply to JSON reply
class ReplyTranslater
{
public:
	ReplyTranslater(const std::string& reply) { _reply = reply; }
	std::string ToJsonReply();

private:
	std::string _reply;

	//commands
	const std::string strCommandDevicesGet = "devices get";
	const std::string strCommandDeviceConnect = "device connect";
	const std::string strCommandDeviceQueryPower = "device query power";
	const std::string strCommandDeviceQueryFuse = "device query fuse";
	const std::string strCommandDeviceDelay = "device delay";
	const std::string strCommandOptPowerOn = "opt power on";
	const std::string strCommandOptPowerOff = "opt power off";
	const std::string strCommandOptQueryPower = "opt query power";
	const std::string strCommandDcmPowerOn = "dcm power on";
	const std::string strCommandDcmPowerOff = "dcm power off";
	const std::string strCommandDcmQueryPower = "dcm query power";
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
	const std::string strCommandStepperQuery = "stepper query";
	const std::string strCommandStepperSetState = "stepper set state";
	const std::string strCommandLocatorQuery = "locator query";
	//events
	const std::string strEventMainPowerOn = "main power is on";
	const std::string strEventMainPowerOff = "main fuse is off";
	const std::string strEventMainFuseOn = "main fuse is on";
	const std::string strEventMainFuseOff = "main fuse is off";
	const std::string strEventOptPoweredOn = "OPT is powered on";
	const std::string strEventOptPoweredOff = "OPT is powered off";
	const std::string strEventBdcsPoweredOn = "BDCs are powered on";
	const std::string strEventBdcsPoweredOff = "BDCs are powered off";
	const std::string strEventBdcCoast = "BDC coast";
	const std::string strEventBdcReverse = "BDC reverse";
	const std::string strEventBdcForward = "BDC forward";
	const std::string strEventBdcBreak = "BDC break";
	const std::string strEventBdcWrongState = "BDC wrong state";
	const std::string strEventSteppersPoweredOn = "steppers are powered on";
	const std::string strEventSteppersPoweredOff = "steppers are powered off";
	const std::string strEventStepperEnabled = "stepper is enabled";
	const std::string strEventStepperDisabled = "stepper is disabled";
	const std::string strEventStepperForward = "stepper forward";
	const std::string strEventStepperBackward = "stepper backward";
	const std::string strEventStepperUnknownPosition = "stepper unknown position";
	const std::string strEventStepperApproachingHomeLocator = "stepper approach home locator";
	const std::string strEventStepperLeavingHomeLocator = "stepper leave home locator";
	const std::string strEventStepperGoHome = "stepper go home";
	const std::string strEventStepperKnownPosition = "stepper known position";
	const std::string strEventStepperAccelerate = "stepper accelerate";
	const std::string strEventStepperCruise = "stepper cruise";
	const std::string strEventStepperDecelerate = "stepper decelerate";
	const std::string strEventStepperWrongState = "stepper wrong state";
	const std::string strEventLocator = "locator";


	char getDigitalValue(char c);
	unsigned long getHexValue(const std::string& hexString);
	//commands
	std::string formatCmdReply(Poco::JSON::Object::Ptr& replyPtr);
	std::string deviceQueryPower(Poco::JSON::Object::Ptr& replyPtr);
	std::string deviceQueryFuse(Poco::JSON::Object::Ptr& replyPtr);
	std::string deviceDelay(Poco::JSON::Object::Ptr& replyPtr);
	std::string optPowerOn(Poco::JSON::Object::Ptr& replyPtr);
	std::string optPowerOff(Poco::JSON::Object::Ptr& replyPtr);
	std::string optQueryPower(Poco::JSON::Object::Ptr& replyPtr);
	std::string dcmPowerOn(Poco::JSON::Object::Ptr& replyPtr);
	std::string dcmPowerOff(Poco::JSON::Object::Ptr& replyPtr);
	std::string dcmQueryPower(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcsPowerOn(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcsPowerOff(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcsQueryPower(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcCoast(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcReverse(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcForward(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcBreak(Poco::JSON::Object::Ptr& replyPtr);
	std::string bdcQuery(Poco::JSON::Object::Ptr& replyPtr);
	std::string steppersPowerOn(Poco::JSON::Object::Ptr& replyPtr);
	std::string steppersPowerOff(Poco::JSON::Object::Ptr& replyPtr);
	std::string steppersQueryPower(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperQueryResolution(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperConfigStep(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperAccelerationBuffer(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperAccelerationBufferDecrement(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperDecelerationBuffer(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperDecelerationBufferIncrement(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperEnable(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperForward(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperSteps(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperRun(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperConfigHome(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperQuery(Poco::JSON::Object::Ptr& replyPtr);
	std::string stepperSetState(Poco::JSON::Object::Ptr& replyPtr);
	std::string locatorQuery(Poco::JSON::Object::Ptr& replyPtr);
	//events
	std::string formatEvent(Poco::JSON::Object::Ptr& replyPtr);
};



#endif /* REPLYTRANSLATER_H_ */
