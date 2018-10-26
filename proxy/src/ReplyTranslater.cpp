/*
 * ReplyTranslater.cpp
 *
 *  Created on: Oct 26, 2018
 *      Author: mikez
 */
#include "ReplyTranslater.h"
#include "Poco/Dynamic/Struct.h"
#include "ProxyLogger.h"

extern ProxyLogger * pLogger;

char ReplyTranslater::getDigitalValue(char c)
{
	char rc;

	switch(c)
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		rc = c - '0';
		break;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
		rc = c - 'a' + 10;
		break;
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
		rc = c - 'A' + 10;
		break;

	default:
		rc = -1;
	}

	return rc;
}

unsigned long ReplyTranslater::getHexValue(const std::string& hexString)
{
	unsigned long value = 0;

	for(int i=0; i<hexString.size(); i++)
	{
		char c = getDigitalValue(hexString[i]);

		if(c < 0) {
			throw Poco::JSON::JSONException("Invalid command value");
		}
		else {
			value = (value << (i * 4)) + c;
		}
	}

	return value;
}

std::string ReplyTranslater::deviceQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::deviceQueryPower wrong parameter amount: " + std::to_string(size));
	}

	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);
	state = ds["state"].toString();

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDeviceQueryPower + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcsPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOn wrong parameter amount: " + std::to_string(size));
	}

	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcsPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOff wrong parameter amount: " + std::to_string(size));
	}

	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcsQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;

	//parameters
	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOn wrong parameter amount: " + std::to_string(size));
	}
	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsQueryPower + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: bdcCoast(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcCoast wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	devCmdId = ds["params"][1].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcCoast + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: bdcReverse(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcReverse wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	devCmdId = ds["params"][1].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcReverse + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: bdcForward(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcForward wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	devCmdId = ds["params"][1].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcForward + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: bdcBreak(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcBreak wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	devCmdId = ds["params"][1].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcBreak + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: bdcQuery(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	std::string state;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcQuery wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	devCmdId = ds["params"][1].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcQuery + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: steppersPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::steppersPowerOn wrong parameter amount: " + std::to_string(size));
	}

	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandSteppersPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: steppersPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string devCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::steppersPowerOff wrong parameter amount: " + std::to_string(size));
	}

	devCmdId = ds["params"][0].toString();
	commandId = getHexValue(devCmdId);

	if(replyPtr->has("state")) {
		state = ds["state"].toString();
	}
	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandSteppersPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!state.empty()) {
		reply = reply + ",\"state\":\"" + state + "\"";
	}
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater:: steppersQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperQueryResolution(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperConfigStep(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperAccelerationBuffer(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperAccelerationBufferDecrement(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperDecelerationBuffer(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperDecelerationBufferIncrement(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperEnable(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperForward(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperSteps(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperRun(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperConfigHome(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: stepperQuery(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater:: locatorQuery(Poco::JSON::Object::Ptr& replyPtr)
{

}



std::string ReplyTranslater::formatCmdReply(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;

	std::string hexCmd = replyPtr->getValue<std::string>("command");
	long cmdValue = getHexValue(hexCmd);

	switch(cmdValue)
	{
	case 2:
		reply = deviceQueryPower(replyPtr);
		break;

	case 40:
		reply = bdcsPowerOn(replyPtr);
		break;

	case 41:
		reply = bdcsPowerOff(replyPtr);
		break;

	case 42:
		reply = bdcsQueryPower(replyPtr);
		break;

	case 43:
		reply = bdcCoast(replyPtr);
		break;

	case 44:
		reply = bdcReverse(replyPtr);
		break;

	case 45:
		reply = bdcForward(replyPtr);
		break;

	case 46:
		reply = bdcBreak(replyPtr);
		break;

	case 47:
		reply = bdcQuery(replyPtr);
		break;

	case 13:
		reply = steppersPowerOn(replyPtr);
		break;

	case 14:
		reply = steppersPowerOff(replyPtr);
		break;

	case 15:
		reply = steppersQueryPower(replyPtr);
		break;

	case 50:
		reply = stepperQueryResolution(replyPtr);
		break;

	case 51:
		reply = stepperConfigStep(replyPtr);
		break;

	case 52:
		reply = stepperAccelerationBuffer(replyPtr);
		break;

	case 53:
		reply = stepperAccelerationBufferDecrement(replyPtr);
		break;

	case 54:
		reply = stepperDecelerationBuffer(replyPtr);
		break;

	case 55:
		reply = stepperDecelerationBufferIncrement(replyPtr);
		break;

	case 56:
		reply = stepperEnable(replyPtr);
		break;

	case 57:
		reply = stepperForward(replyPtr);
		break;

	case 58:
		reply = stepperSteps(replyPtr);
		break;

	case 59:
		reply = stepperRun(replyPtr);
		break;

	case 60:
		reply = stepperConfigHome(replyPtr);
		break;

	case 61:
		reply = stepperQuery(replyPtr);
		break;

	case 100:
		reply = locatorQuery(replyPtr);
		break;

	default:
		//empty reply is returned.
		break;
	}

	return reply;
}

std::string ReplyTranslater::formatEvent(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string event;

	return event;
}


std::string ReplyTranslater::ToJsonReply()
{
	std::string formatedReply;

	try
	{
		Poco::JSON::Parser parser;
		Poco::Dynamic::Var result = parser.parse(_reply);
		Poco::JSON::Object::Ptr replyPtr = result.extract<Poco::JSON::Object::Ptr>();

		if(replyPtr->has("command")) {
			formatedReply = formatCmdReply(replyPtr);
		}
		else if(replyPtr->has("event")) {
			formatedReply = formatEvent(replyPtr);
		}
		else {
			pLogger->LogError("ReplyTranslater::ToJsonReply invalid reply: " + _reply);
		}
	}
	catch(Poco::JSON::JSONException& e)
	{
		pLogger->LogError("ReplyTranslater::ToJsonReply exception occurs: " + e.displayText() + " in " + _reply);
	}
	catch(...)
	{
		pLogger->LogError("ReplyTranslater::ToJsonReply unknown exception in " + _reply);
	}

	return formatedReply;
}


