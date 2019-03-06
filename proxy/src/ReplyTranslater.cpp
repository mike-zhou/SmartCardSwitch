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

	for(unsigned int i=0; i<hexString.size(); i++)
	{
		char c = getDigitalValue(hexString[i]);

		if(c < 0) {
			throw Poco::JSON::JSONException("Invalid command value");
		}
		else {
			value = (value << 4) + c;
		}
	}

	return value;
}

std::string ReplyTranslater::deviceQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::deviceQueryPower wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);
	state = ds["state"].toString();

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		state = ds["state"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDeviceQueryPower + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	else {
		reply = reply + ",\"state\":\"" + state + "\"";
		//"state":"powered on"
		//"state":"powered off"
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::deviceQueryFuse(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::deviceQueryFuse wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);
	state = ds["state"].toString();

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		state = ds["state"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDeviceQueryFuse + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	else {
		reply = reply + ",\"state\":\"" + state + "\"";
		//"\"main fuse is on\""
		//"\"main fuse is off\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::deviceDelay(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::deviceDelay wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDeviceDelay + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::optPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::optPowerOn wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandOptPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::optPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::optPowerOff wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandOptPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::optQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater::dcmPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long index;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::dcmPowerOn wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDcmPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	reply = reply + ",\"index\":" + std::to_string(index);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::dcmPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long index;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::dcmPowerOff wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandDcmPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	reply = reply + ",\"index\":" + std::to_string(index);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::dcmQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{

}

std::string ReplyTranslater::bdcsPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOn wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcsPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOff wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcsQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string state;
	std::string error;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;

	//parameters
	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcsPowerOn wrong parameter amount: " + std::to_string(size));
	}
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		state = ds["state"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcsQueryPower + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	else {
		reply = reply + ",\"state\":\"" + state + "\"";
		//"\"state\":\"BDCs are powered on\""
		//"\"state\":\"BDCs are powered off\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcCoast(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
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
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcCoast + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"BDC index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcReverse(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 5) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcReverse wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcReverse + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"BDC index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcForward(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 5) {
		throw Poco::JSON::JSONException("ReplyTranslater::bdcForward wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcForward + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"BDC index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcBreak(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
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
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcBreak + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"BDC index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::bdcQuery(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
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
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		state = ds["state"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandBdcQuery + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"BDC index is out of scope\""
		//"\"error\":\"wrong BDC state\""
	}
	else {
		reply = reply + ",\"state\":\"" + state + "\"";
		//"\"state\":\"coast\""
		//"\"state\":\"reverse\""
		//"\"state\":\"forward\""
		//"\"state\":\"break\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::steppersPowerOn(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::steppersPowerOn wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandSteppersPowerOn + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::steppersPowerOff(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::steppersPowerOff wrong parameter amount: " + std::to_string(size));
	}

	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandSteppersPowerOff + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::steppersQueryPower(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string state;
	std::string error;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::steppersQueryPower wrong parameter amount: " + std::to_string(size));
	}
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		state = ds["state"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandSteppersQueryPower + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	else {
		reply = reply + ",\"state\":\"" + state + "\"";
		//"\"state\":\"steppers are powered on\""
		//"\"state\":\"steppers are powered off\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperQueryResolution(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	std::string error;
	std::string strResolution;
	unsigned long resolution;

	auto size = ds["params"].size();
	if(size != 1) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperQueryResolution wrong parameter amount: " + std::to_string(size));
	}
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		strResolution = ds["resolution"].toString();
		resolution = getHexValue(strResolution);
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperQueryResolution + "\",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	else {
		reply = reply + ",\"resolution\":" + std::to_string(resolution);
		//"\"resolution\":\"89ab\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperConfigStep(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 4) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperConfigStep wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperConfigStep + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperAccelerationBuffer(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperAccelerationBuffer wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperAccelerationBuffer + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperAccelerationBufferDecrement(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperAccelerationBufferDecrement wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperAccelerationBufferDecrement + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperDecelerationBuffer(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperDecelerationBuffer wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperDecelerationBuffer + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperDecelerationBufferIncrement(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperDecelerationBufferIncrement wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperDecelerationBufferIncrement + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperEnable(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;
	std::string strEnable;
	bool enable;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperEnable wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strEnable = ds["params"][1].toString();
	enable = (getHexValue(strEnable) != 0) ? true : false;
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperEnable + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"enabled\":" + (enable ? "true" : "false") + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperForward(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;
	std::string strForward;
	bool forward;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperForward wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strForward = ds["params"][1].toString();
	forward = (getHexValue(strForward) != 0) ? true : false;
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperForward + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"forward\":" + (forward ? "true" : "false") + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperSteps(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperSteps wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperSteps + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
		//"\"error\":\"stepper has not been positioned\""
		//"\"error\":\"invalid parameter\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperRun(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;
	std::string strPosition;
	long position;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperRun wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}
	else {
		strPosition = ds["position"].toString();
		position = getHexValue(strPosition);
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperRun + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	else {
		reply = reply + ",\"position\":" + std::to_string(position);
		//"position":"1234"
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperConfigHome(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 5) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperConfigHome wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperConfigHome + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
		//"\"error\":\"locator index is out of scope\""
		//"\"error\":\"locator line index is out of scope\""
		//"\"error\":\"duplicated locator line index\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperQuery(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;
	//content queried.
	std::string stepperState;
	std::string strEnabled;
	bool enabled;
	std::string strForward;
	bool forward;
	std::string strLocatorIndex;
	long locatorIndex;
	std::string strLocatorLineNumberStart;
	long locatorLineNumberStart;
	std::string strLocatorLineNumberTerminal;
	long locatorLineNumberTerminal;
	std::string strHomeOffset;
	long homeOffset;
	std::string strLowClks;
	long lowClks;
	std::string strHighClks;
	long highClks;
	std::string strAccelerationBuffer;
	long accelerationBuffer;
	std::string strAccelerationBufferDecrement;
	long accelerationBufferDecrement;
	std::string strDecelerationBuffer;
	long decelerationBuffer;
	std::string strDecelerationBufferIncrement;
	long decelerationBufferIncrement;


	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperQuery wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
	}
	else
	{
		stepperState = ds["state"].toString();
		strEnabled = ds["enabled"].toString();
		strForward = ds["forward"].toString();
		strLocatorIndex = ds["locatorIndex"].toString();
		strLocatorLineNumberStart = ds["locatorLineNumberStart"].toString();
		strLocatorLineNumberTerminal = ds["locatorLineNumberTerminal"].toString();
		strHomeOffset = ds["homeOffset"].toString();
		strLowClks = ds["lowClks"].toString();
		strHighClks = ds["highClks"].toString();
		strAccelerationBuffer = ds["accelerationBuffer"].toString();
		strAccelerationBufferDecrement = ds["accelerationDecrement"].toString();
		strDecelerationBuffer = ds["decelerationBuffer"].toString();
		strDecelerationBufferIncrement = ds["decelerationIncrement"].toString();

		enabled = (strEnabled != "0") ? true : false;
		forward = (strForward != "0") ? true : false;
		locatorIndex = getHexValue(strLocatorIndex);
		locatorLineNumberStart = getHexValue(strLocatorLineNumberStart);
		locatorLineNumberTerminal = getHexValue(strLocatorLineNumberTerminal);
		homeOffset = getHexValue(strHomeOffset);
		lowClks = getHexValue(strLowClks);
		highClks = getHexValue(strHighClks);
		accelerationBuffer = getHexValue(strAccelerationBuffer);
		accelerationBufferDecrement = getHexValue(strAccelerationBufferDecrement);
		decelerationBuffer = getHexValue(strDecelerationBuffer);
		decelerationBufferIncrement = getHexValue(strDecelerationBufferIncrement);
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperQuery + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(error.empty())
	{
		reply = reply + ",\"state\":\"" + stepperState + "\"";
		reply = reply + ",\"enabled\":" + (enabled ? "true" : "false");
		reply = reply + ",\"forward\":" + (forward ? "true" : "false");
		reply = reply + ",\"locatorIndex\":" + std::to_string(locatorIndex);
		reply = reply + ",\"locatorLineNumberStart\":" + std::to_string(locatorLineNumberStart);
		reply = reply + ",\"locatorLineNumberTerminal\":" + std::to_string(locatorLineNumberTerminal);
		reply = reply + ",\"homeOffset\":" + std::to_string(homeOffset);
		reply = reply + ",\"lowClks\":" + std::to_string(lowClks);
		reply = reply + ",\"highClks\":" + std::to_string(highClks);
		reply = reply + ",\"accelerationBuffer\":" + std::to_string(accelerationBuffer);
		reply = reply + ",\"accelerationBufferDecrement\":" + std::to_string(accelerationBufferDecrement);
		reply = reply + ",\"decelerationBuffer\":" + std::to_string(decelerationBuffer);
		reply = reply + ",\"decelerationBufferIncrement\":" + std::to_string(decelerationBufferIncrement);
	}
	else {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::stepperSetState(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;

	//parameters
	auto size = ds["params"].size();
	if(size != 3) {
		throw Poco::JSON::JSONException("ReplyTranslater::stepperSetState wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandStepperSetState + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"stepper index is out of scope\""
		//"\"error\":\"stepper has not been positioned\""
		//"\"error\":\"invalid parameter\""
	}
	reply += "}";

	return reply;
}

std::string ReplyTranslater::locatorQuery(Poco::JSON::Object::Ptr& replyPtr)
{
	std::string reply;
	std::string strCmdId;
	std::string error;
	std::string strIndex;
	Poco::DynamicStruct ds = *replyPtr;
	long commandId;
	long index;
	std::string strLowInput;
	long lowInput;

	//parameters
	auto size = ds["params"].size();
	if(size != 2) {
		throw Poco::JSON::JSONException("ReplyTranslater::locatorQuery wrong parameter amount: " + std::to_string(size));
	}
	strIndex = ds["params"][0].toString();
	index = getHexValue(strIndex);
	strCmdId = ds["params"][size - 1].toString();
	commandId = getHexValue(strCmdId);

	if (replyPtr->has("error")) {
		error = ds["error"].toString();
		//"\"error\":\"invalid command\""
		//"\"error\":\"too many parameters\""
		//"\"error\":\"unknown command\""
		//"\"error\":\"wrong parameter amount\""
		//"\"error\":\"locator line index is out of scope\""
	}
	else {
		strLowInput = ds["lowInput"].toString();
		lowInput = getHexValue(strLowInput);
	}

	reply = "{";
	reply = reply + "\"command\":\"" + strCommandLocatorQuery + "\",";
	reply = reply + "\"index\":" + std::to_string(index) + ",";
	reply = reply + "\"commandId\":" + std::to_string(commandId);
	if(!error.empty()) {
		reply = reply + ",\"error\":\"" + error + "\"";
	}
	else {
		reply = reply + ",\"lowInput\":" + std::to_string(lowInput);
	}
	reply += "}";

	return reply;
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

	case 3:
		reply = deviceQueryFuse(replyPtr);
		break;

	case 4:
		reply = deviceDelay(replyPtr);
		break;

	case 10:
		reply = optPowerOn(replyPtr);
		break;

	case 11:
		reply = optPowerOff(replyPtr);
		break;

	case 20:
		reply = steppersPowerOn(replyPtr);
		break;

	case 21:
		reply = steppersPowerOff(replyPtr);
		break;

	case 22:
		reply = steppersQueryPower(replyPtr);
		break;

	case 30:
		reply = dcmPowerOn(replyPtr);
		break;

	case 31:
		reply = dcmPowerOff(replyPtr);
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

	case 62:
		reply = stepperSetState(replyPtr);
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
	Poco::DynamicStruct ds = *replyPtr;
	std::string strEvent = ds["event"].toString();

	event = "{\"event\":\"";

	if(strEvent == strEventMainPowerOn) {
		event += strEventMainPowerOn + "\"";
	}
	else if (strEvent == strEventMainPowerOff) {
		event += strEventMainPowerOff + "\"";
	}
	else if (strEvent == strEventOptPoweredOn) {
		event += strEventOptPoweredOn + "\"";
	}
	else if (strEvent == strEventOptPoweredOff) {
		event += strEventOptPoweredOff + "\"";
	}
	else if (strEvent == strEventBdcsPoweredOn) {
		event += strEventBdcsPoweredOn + "\"";
	}
	else if (strEvent == strEventBdcsPoweredOff) {
		event += strEventBdcsPoweredOff + "\"";
	}
	else if (strEvent == strEventBdcCoast) {
		event += strEventBdcCoast + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventBdcReverse) {
		event += strEventBdcReverse + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventBdcForward) {
		event += strEventBdcForward + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventBdcBreak) {
		event += strEventBdcBreak + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventBdcWrongState) {
		event += strEventBdcWrongState + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventSteppersPoweredOn) {
		event += strEventSteppersPoweredOn + "\"";
	}
	else if (strEvent == strEventSteppersPoweredOff) {
		event += strEventSteppersPoweredOff + "\"";
	}
	else if (strEvent == strEventStepperEnabled) {
		event += strEventStepperEnabled + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperDisabled) {
		event += strEventStepperDisabled + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperForward) {
		event += strEventStepperForward + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperBackward) {
		event += strEventStepperBackward + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperUnknownPosition) {
		event += strEventStepperUnknownPosition + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperApproachingHomeLocator) {
		event += strEventStepperApproachingHomeLocator + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperLeavingHomeLocator) {
		event += strEventStepperLeavingHomeLocator + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperGoHome) {
		event += strEventStepperGoHome + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperKnownPosition) {
		event += strEventStepperKnownPosition + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperAccelerate) {
		event += strEventStepperAccelerate + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperCruise) {
		event += strEventStepperCruise + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperDecelerate) {
		event += strEventStepperDecelerate + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventStepperWrongState) {
		event += strEventStepperWrongState + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);
	}
	else if (strEvent == strEventLocator) {
		event += strEventLocator + "\"";

		std::string strIndex = ds["index"].toString();
		long index = getHexValue(strIndex);
		event = event + ",\"index\":" + std::to_string(index);

		std::string strInput = ds["input"].toString();
		long input = getHexValue(strInput);
		event = event + ",\"input\":" + std::to_string(input);
	}
	else {
		event = event + "unknown event\"";
	}

	event += "}";

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
	//if any exception in reply parsing, program jumps here, then an empty reply is returned.
	catch(Poco::Exception& e)
	{
		pLogger->LogError("ReplyTranslater::ToJsonReply exception occurs: " + e.displayText() + " in " + _reply);
	}
	catch(...)
	{
		pLogger->LogError("ReplyTranslater::ToJsonReply unknown exception in " + _reply);
	}

	return formatedReply;
}


