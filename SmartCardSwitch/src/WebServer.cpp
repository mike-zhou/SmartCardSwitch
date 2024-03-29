/*
 * WebServer.cpp
 *
 *  Created on: Jan 3, 2019
 *      Author: mikez
 */
#include "Poco/ThreadPool.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/Dynamic/Struct.h"

#include "WebServer.h"
#include "Logger.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

extern Logger * pLogger;
extern CoordinateStorage * pCoordinateStorage;
extern MovementConfiguration * pMovementConfiguration;

std::string ScsRequestHandler::getJsonCommand(Poco::Net::HTTPServerRequest& request)
{
	auto& iStream = request.stream();
	std::string json;

	//read body of request
	for(;;)
	{
		char c;

		iStream.read(&c, 1);
		if(iStream.eof()) {
			break;
		}
		else if(iStream.bad()) {
			pLogger->LogError("ScsRequestHandler::getJsonCommand stream bad");
			break;
		}
		else if(iStream.fail()) {
			pLogger->LogError("ScsRequestHandler::getJsonCommand stream fail");
			break;
		}

		json.push_back(c);
	}

	return json;
}

void ScsRequestHandler::onDefaultHtml(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	auto defaultPage = _pWebServer->GetDefaultPageContent();

	response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
	response.setContentType("text/html");
	response.setChunkedTransferEncoding(true);

	std::ostream& ostr = response.send();
	ostr << defaultPage;
}

void ScsRequestHandler::onStepperMove(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onStepperMove no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onStepperMove command: " + command);
		unsigned int stepperIndex;
		bool forward;
		unsigned int steps;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			stepperIndex = ds["index"];
			forward = ds["forward"];
			steps = ds["steps"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onStepperMove exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onStepperMove unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onStepperMove reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			if(!_pWebServer->StepperMove(stepperIndex, forward, steps, errorInfo))
			{
				pLogger->LogError("ScsRequestHandler::onStepperMove failed to move stepper: " + errorInfo);
			}

			if(errorInfo.empty())
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}

	pLogger->LogInfo("ScsRequestHandler::onStepperMove request has been processed");
}

void ScsRequestHandler::onQuery(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string errorInfo;

	if(!_pWebServer->Query(errorInfo)) {
		pLogger->LogError("ScsRequestHandler::onQuery failed: " + errorInfo);
		response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
		response.setReason(errorInfo);
		response.send();
		return;
	}

	response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
	response.setContentType("application/json");
	auto& oStream = response.send();
	oStream << _pWebServer->DeviceStatus();
}

void ScsRequestHandler::onBdc(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onBdc no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onBdc command: " + command);
		unsigned int bdcIndex;
		unsigned int action;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			bdcIndex = ds["index"];
			action = ds["action"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onBdc exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onBdc unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onBdc reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else if(action > 2)
		{
			pLogger->LogError("ScsRequestHandler::onBdc wrong action in command");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in action: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			switch(action)
			{
				case 0:	//forward
				{
					if(!_pWebServer->BdcForward(bdcIndex, errorInfo))
					{
						pLogger->LogError("ScsRequestHandler::onBdc failed to forward bdc: " + errorInfo);
					}
				}
				break;

				case 1: //backward
				{
					if(!_pWebServer->BdcReverse(bdcIndex, errorInfo))
					{
						pLogger->LogError("ScsRequestHandler::onBdc failed to reverse bdc: " + errorInfo);
					}
				}
				break;

				case 2: //deactivate
				{
					if(!_pWebServer->BdcDeactivate(bdcIndex, errorInfo))
					{
						pLogger->LogError("ScsRequestHandler::onBdc failed to deactivate bdc: " + errorInfo);
					}
				}
				break;

				default:
				{
					errorInfo = "wrong bdc action: " + std::to_string(action);
					pLogger->LogError("ScsRequestHandler::onBdc wrong bdc action: " + std::to_string(action));
				}
				break;
			}

			if(errorInfo.empty())
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}

	pLogger->LogInfo("ScsRequestHandler::onBdc request has been processed");
}

void ScsRequestHandler::onStepperConfigMovement(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onStepperConfigMovement no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onStepperConfigMovement command: " + command);
		unsigned int index;
		unsigned int lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			index = ds["index"];
			lowClks = ds["lowClks"];
			highClks = ds["highClks"];
			accelerationBuffer = ds["accelerationBuffer"];
			accelerationBufferDecrement = ds["accelerationBufferDecrement"];
			decelerationBuffer = ds["decelerationBuffer"];
			decelerationBufferIncrement = ds["decelerationBufferIncrement"];

			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigMovement exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigMovement unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigMovement reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;
			if(!_pWebServer->StepperConfigMovement(index, lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement, errorInfo))
			{
				pLogger->LogError("ScsRequestHandler::onStepperConfigMovement failed to configure stepper movement: " + errorInfo);
			}

			if(errorInfo.empty()) {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onStepperConfigForwardClockwise(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onStepperConfigForwardClockwise no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onStepperConfigForwardClockwise command: " + command);
		unsigned int index;
		bool forwardClockwise;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			index = ds["index"];
			forwardClockwise = ds["forwardClockwise"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigForwardClockwise exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigForwardClockwise unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigForwardClockwise reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;
			if(!_pWebServer->StepperConfigForwardClockwise(index, forwardClockwise, errorInfo))
			{
				pLogger->LogError("ScsRequestHandler::onStepperConfigForwardClockwise failed to configure stepper movement: " + errorInfo);
			}

			if(errorInfo.empty()) {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onStepperConfigHome(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onStepperConfigHome no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onStepperConfigHome command: " + command);
		unsigned int index;
		unsigned int locator, locatorLineNumberStart, locatorLineNumberTerminal;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			index = ds["index"];
			locator = ds["locator"];
			locatorLineNumberStart = ds["locatorLineNumberStart"];
			locatorLineNumberTerminal = ds["locatorLineNumberTerminal"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigHome exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigHome unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onStepperConfigHome reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;
			if(!_pWebServer->StepperConfigHome(index, locator, locatorLineNumberStart, locatorLineNumberTerminal, errorInfo))
			{
				pLogger->LogError("ScsRequestHandler::onStepperConfigHome failed to configure stepper movement: " + errorInfo);
			}

			if(errorInfo.empty()) {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onToCoordinate(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onToCoordinate no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onToCoordinate command: " + command);
		unsigned int x, y, z, w;
		bool direct;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			x = ds["x"];
			y = ds["y"];
			z = ds["z"];
			w = ds["w"];
			direct = ds["direct"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinate exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinate unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinate reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;
			if(direct) {
				if(!_pWebServer->ToCoordinate(x, y, z, w, errorInfo)) {
					pLogger->LogError("ScsRequestHandler::onToCoordinate failed: " + errorInfo);
				}
			}
			else {
				if(!_pWebServer->ToCoordinateIndirect(x, y, z, w, errorInfo)) {
					pLogger->LogError("ScsRequestHandler::onToCoordinate failed: " + errorInfo);
				}
			}

			if(errorInfo.empty()) {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onToCoordinateItem(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onToCoordinateItem no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onToCoordinateItem command: " + command);
		int x, y, z, w, v;
		bool exceptionOccurred = true;

		x = -1;
		y = -1;
		z = -1;
		w = -1;
		v = -1;
		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			if(objectPtr->has("x")) {
				x = ds["x"];
			}
			else if(objectPtr->has("y")) {
				y = ds["y"];
			}
			else if(objectPtr->has("z")) {
				z = ds["z"];
			}
			else if(objectPtr->has("w")) {
				w = ds["w"];
			}
			else if(objectPtr->has("v")) {
				v = ds["v"];
			}
			else {
				throw Poco::Exception("wrong parameter in command");
			}

			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinateItem exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinateItem unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onToCoordinateItem reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;
			if(x >= 0) {
				_pWebServer->ToCoordinateItem(0, x, errorInfo);
			}
			else if(y >= 0) {
				_pWebServer->ToCoordinateItem(1, y, errorInfo);
			}
			else if(z >= 0) {
				_pWebServer->ToCoordinateItem(2, z, errorInfo);
			}
			else if(w >= 0) {
				_pWebServer->ToCoordinateItem(3, w, errorInfo);
			}
			else if(v >= 0) {
				_pWebServer->ToCoordinateItem(4, v, errorInfo);
			}

			if(errorInfo.empty()) {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				auto& oStream = response.send();
				oStream << _pWebServer->DeviceStatus();
			}
			else {
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onSaveCoordinate(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onSaveCoordinate no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onSaveCoordinate command: " + command);
		std::string coordinateType;
		unsigned int data;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			coordinateType = ds["coordinateType"].toString();
			data = ds["data"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onSaveCoordinate exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onSaveCoordinate unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onSaveCoordinate reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			if(!_pWebServer->SaveCoordinate(coordinateType, data, errorInfo)) {
				pLogger->LogError("ScsRequestHandler::onSaveCoordinate failed: " + errorInfo);
			}

			if(errorInfo.empty())
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				response.send();
			}
			else
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}

	pLogger->LogInfo("ScsRequestHandler::onSaveCoordinate request has been processed");
}

void ScsRequestHandler::onPower(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onPower no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onPower command: " + command);
		std::string target;
		bool on;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			target = ds["target"].toString();
			on = ds["on"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onPower exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onPower unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onPower reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			if(!_pWebServer->PowerOn(target, on, errorInfo)) {
				pLogger->LogError("ScsRequestHandler::onPower failed: " + errorInfo);
			}

			if(errorInfo.empty())
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				response.send();
			}
			else
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::onToSmartCardOffset(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	std::string command = getJsonCommand(request);

	//execute command
	if(command.empty())
	{
		pLogger->LogError("ScsRequestHandler::onToSmartCardOffset no command in request");

		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("no command in request");
		response.send();
	}
	else
	{
		pLogger->LogInfo("ScsRequestHandler::onToSmartCardOffset command: " + command);
		unsigned int offset;
		bool exceptionOccurred = true;

		try
		{
			Poco::JSON::Parser parser;
			Poco::Dynamic::Var result = parser.parse(command);
			Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();
			Poco::DynamicStruct ds = *objectPtr;

			offset = ds["v"];
			exceptionOccurred = false;
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("ScsRequestHandler::onToSmartCardOffset exception: " + e.displayText());
		}
		catch(...)
		{
			pLogger->LogError("ScsRequestHandler::onToSmartCardOffset unknown exception occurred");
		}

		//reply to request
		if(exceptionOccurred)
		{
			pLogger->LogError("ScsRequestHandler::onToSmartCardOffset reply bad request to browser");

			response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
			response.setReason("wrong parameter in: " + command);
			response.send();
		}
		else
		{
			std::string errorInfo;

			if(!_pWebServer->ToSmartCardOffset(offset, errorInfo)) {
				pLogger->LogError("ScsRequestHandler::onToSmartCardOffset failed: " + errorInfo);
			}

			if(errorInfo.empty())
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
				response.setContentType("application/json");
				response.send();
			}
			else
			{
				response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
				response.setReason(errorInfo);
				response.send();
			}
		}
	}
}

void ScsRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	const std::string& uri = request.getURI();
	pLogger->LogInfo("ScsRequestHandler::handleRequest %%%%%% URI: " + uri);

	if(uri == "/") {
		onDefaultHtml(request, response);
	}
	else if(uri == "/stepperMove") {
		onStepperMove(request, response);
	}
	else if(uri == "/stepperConfigMovement") {
		onStepperConfigMovement(request, response);
	}
	else if(uri == "/stepperConfigHome") {
		onStepperConfigHome(request, response);
	}
	else if(uri == "/query") {
		onQuery(request, response);
	}
	else if(uri == "/bdc") {
		onBdc(request, response);
	}
	else if(uri == "/saveCoordinate") {
		onSaveCoordinate(request, response);
	}
	else if(uri == "/toCoordinate") {
		onToCoordinate(request, response);
	}
	else if(uri == "/toCoordinateItem") {
		onToCoordinateItem(request, response);
	}
	else if(uri == "/power") {
		onPower(request, response);
	}
	else if(uri == "/toSmartCardOffset") {
		onToSmartCardOffset(request, response);
	}
	else
	{
		response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
		response.setReason("Bad request");
		response.send();
	}
}

/**
 *  WebServer
 */

WebServer::WebServer(unsigned int port, unsigned int maxQueue, unsigned int maxThread, const std::string & filesFolder): Task("WebServer")
{
	_port = port;
	_maxQueue = maxQueue;
	_maxThread = maxThread;
	_filesFolder = filesFolder;
	_pConsoleOperator = nullptr;

	_consoleCommand.state  = CommandState::Idle;
	_consoleCommand.cmdId = ICommandReception::ICommandDataTypes::InvalidCommandId;
	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		auto& data = _consoleCommand.resultSteppers[i];

		data.maximum = 0;
		data.homeOffset = 0;
	}
}


const std::string& WebServer::GetDefaultPageContent()
{
	//if(_defaultPageContent.empty()) //commented to read default page every time.
	{
		Poco::Path pagePath(_filesFolder);
		pagePath.setFileName("default.html");

		_defaultPageContent.clear();

		Poco::File defaultFile(pagePath.toString());

		if(defaultFile.exists())
		{
			std::string filePathName = pagePath.toString();

			FILE * fd = fopen(filePathName.c_str(), "r");
			if(fd < 0) {
				pLogger->LogError("WebServer::GetDefaultPageContent cannot open file: " + filePathName);
			}
			else
			{
				for(;;)
				{
					unsigned char c;
					auto amount = fread(&c, 1, 1, fd);
					if(amount < 1) {
						break;
					}
					else {
						_defaultPageContent.push_back(c);
					}
				}
				//close file
				fclose(fd);
			}
		}

		if(_defaultPageContent.empty())
		{
			pLogger->LogError("WebServer::GetDefaultPageContent cannot read from: " + pagePath.toString());
			_defaultPageContent = "<html><head><title>SmartCardSwitch Config Server</title></head>";
			_defaultPageContent += "<body><p style=\"text-align: center; font-size: 48px;\">";
			_defaultPageContent += "internal error";
			_defaultPageContent += "</p></body></html>";
		}
	}

	return _defaultPageContent;
}

void WebServer::OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices)
{

}

void WebServer::OnDeviceConnect(CommandId key, bool bSuccess)
{

}

void WebServer::OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultDevicePowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}

}

void WebServer::OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultDeviceFuseOk = bFuseOn;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnDeviceDelay(CommandId key, bool bSuccess)
{

}

void WebServer::OnOptPowerOn(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppersPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnOptPowerOff(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppersPowered = false;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultOptPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnDcmPowerOn(CommandId key, bool bSuccess)
{

}

void WebServer::OnDcmPowerOff(CommandId key, bool bSuccess)
{

}

void WebServer::OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcsPowerOn(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcsPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcsPowerOff(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcsPowered = false;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcsPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcCoast(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcStatus[_consoleCommand.bdcIndex] = _consoleCommand.bdcStatus;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcReverse(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcStatus[_consoleCommand.bdcIndex] = _consoleCommand.bdcStatus;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcForward(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcStatus[_consoleCommand.bdcIndex] = _consoleCommand.bdcStatus;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcBreak(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcStatus[_consoleCommand.bdcIndex] = _consoleCommand.bdcStatus;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultBdcStatus[_consoleCommand.bdcIndex] = status;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnSteppersPowerOn(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppersPowered = true;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnSteppersPowerOff(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppersPowered = false;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppersPowered = bPowered;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs)
{

}

void WebServer::OnStepperConfigStep(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		auto& data = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];

		data.lowClks = _consoleCommand.lowClks;
		data.highClks = _consoleCommand.highClks;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperAccelerationBuffer(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].accelerationBuffer = _consoleCommand.accelerationBuffer;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].accelerationBufferDecrement = _consoleCommand.accelerationBufferDecrement;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperDecelerationBuffer(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].decelerationBuffer = _consoleCommand.decelerationBuffer;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].decelerationBufferIncrement = _consoleCommand.decelerationBufferIncrement;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperEnable(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperForward(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		//update result.
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forward = _consoleCommand.stepperForward;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperSteps(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperRun(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		//update result
		auto& data = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];
		if(data.forward) {
			data.homeOffset += _consoleCommand.steps;
		}
		else {
			data.homeOffset -= _consoleCommand.steps;
		}
		if(data.homeOffset > data.maximum) {
			data.maximum = data.homeOffset;
		}
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperConfigHome(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		//update result
		auto& data = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];

		data.forward = false;
		data.homeOffset = 0;
		data.maximum = 0;
		data.locatorIndex = _consoleCommand.locatorIndex;
		data.locatorLineNumberStart = _consoleCommand.locatorLineNumberStart;
		data.locatorLineNumberTerminal = _consoleCommand.locatorLineNumberTerminal;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperMove(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperQuery(CommandId key, bool bSuccess,
							StepperState state,
							bool bEnabled,
							bool bForward,
							bool bForwardClockwise,
							unsigned int locatorIndex,
							unsigned int locatorLineNumberStart,
							unsigned int locatorLineNumberTerminal,
							unsigned long homeOffset,
							unsigned long lowClks,
							unsigned long highClks,
							unsigned long accelerationBuffer,
							unsigned long accelerationBufferDecrement,
							unsigned long decelerationBuffer,
							unsigned long decelerationBufferIncrement)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		auto& data = _consoleCommand.resultSteppers[_consoleCommand.stepperIndex];

		data.state = state;
		data.forward = bForward;
		data.forwardClockwise = bForwardClockwise;
		data.enabled = bEnabled;
		data.locatorIndex = locatorIndex;
		data.locatorLineNumberStart = locatorLineNumberStart;
		data.locatorLineNumberTerminal = locatorLineNumberTerminal;
		data.homeOffset = homeOffset;
		data.lowClks = lowClks;
		data.highClks = highClks;
		data.accelerationBuffer = accelerationBuffer;
		data.accelerationBufferDecrement = accelerationBufferDecrement;
		data.decelerationBuffer = decelerationBuffer;
		data.decelerationBufferIncrement = decelerationBufferIncrement;
		if(data.homeOffset > data.maximum) {
			data.maximum = data.homeOffset;
		}

		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnStepperSetState(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperForwardClockwise(CommandId key, bool bSuccess)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess) {
		//update result.
		_consoleCommand.resultSteppers[_consoleCommand.stepperIndex].forwardClockwise = _consoleCommand.stepperForwardClockwise;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

void WebServer::OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput)
{
	if(key == InvalidCommandId) {
		return;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

	if(_consoleCommand.state != CommandState::OnGoing) {
		return;
	}
	if(_consoleCommand.cmdId != key) {
		return;
	}

	if(bSuccess)
	{
		_consoleCommand.resultLocators[_consoleCommand.locatorIndex] = lowInput;
		_consoleCommand.state = CommandState::Succeeded;
	}
	else {
		_consoleCommand.state = CommandState::Failed;
	}
}

bool WebServer::StepperMove(unsigned int index, bool forward, unsigned int steps, std::string & errorInfo)
{
	errorInfo.clear();
	std::string cmd;

	if(index >= STEPPER_AMOUNT)
	{
		errorInfo = "stepper index is out of range, index:" + std::to_string(index);
		pLogger->LogError("WebServer::StepperMove " + errorInfo);
		return false;
	}
	if(steps == 0) {
		pLogger->LogInfo("WebServer::StepperMove stepper needn't move");
		return true;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	//check parameters
	{
		Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

		if(_consoleCommand.state != CommandState::Idle) {
			errorInfo = "internal error: wrong command state";
		}
		if(_consoleCommand.resultSteppers[index].state != StepperState::KnownPosition)  {
			errorInfo = "stepper hasn't been positioned";
		}
		if(!forward)
		{
			if(_consoleCommand.resultSteppers[index].homeOffset < steps)  {
				errorInfo = "stepper cannot be moved under scope";
			}
		}
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::StepperMove " + errorInfo);
			return false;
		}
	}

	_consoleCommand.stepperIndex = index;
	_consoleCommand.stepperForward = forward;
	_consoleCommand.steps = steps;

	cmd = ConsoleCommandFactory::CmdStepperForward(index, forward);
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperMove error in stepper forward: " + errorInfo);
		return false;
	}

	cmd = ConsoleCommandFactory::CmdStepperSteps(index, steps);
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperMove error in stepper steps: " + errorInfo);
		return false;
	}

	long finalPos;
	if(forward) {
		finalPos = _consoleCommand.resultSteppers[index].homeOffset + steps;
	}
	else {
		finalPos = _consoleCommand.resultSteppers[index].homeOffset - steps;
	}
	cmd = ConsoleCommandFactory::CmdStepperRun(index, _consoleCommand.resultSteppers[index].homeOffset, finalPos);
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperMove error in stepper steps: " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::StepperConfigHome(
					unsigned int stepperIndex,
					unsigned int locatorIndex,
					unsigned int locatorLineNumberStart,
					unsigned int locatorLineNumberTerminal,
					std::string & errorInfo)
{
	errorInfo.clear();

	if(stepperIndex >= STEPPER_AMOUNT) {
		errorInfo = "stepper index is out of range: " + std::to_string(stepperIndex);
	}
	if(locatorIndex >= LOCATOR_AMOUNT) {
		errorInfo = "locator index is out of range: " + std::to_string(locatorIndex);
	}
	if((locatorLineNumberStart < 1) || (locatorLineNumberStart > 8)) {
		errorInfo = "invalid parameter value";
	}
	if((locatorLineNumberTerminal < 1) || (locatorLineNumberTerminal > 8)) {
		errorInfo = "invalid parameter value";
	}
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement " + errorInfo);
		return false;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	_consoleCommand.stepperIndex = stepperIndex;
	_consoleCommand.locatorIndex = locatorIndex;
	_consoleCommand.locatorLineNumberStart = locatorLineNumberStart;
	_consoleCommand.locatorLineNumberTerminal = locatorLineNumberTerminal;
	_consoleCommand.steps = 0;

	std::string command = ConsoleCommandFactory::CmdStepperConfigHome(stepperIndex, locatorIndex, locatorLineNumberStart, locatorLineNumberTerminal);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigHome failed to configure home : " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdStepperRun(stepperIndex, 0, 0);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigHome failed run stepper : " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::StepperConfigMovement(
					unsigned int index,
					unsigned int lowClks,
					unsigned int highClks,
					unsigned int accelerationBuffer,
					unsigned int accelerationBufferDecrement,
					unsigned int decelerationBuffer,
					unsigned int decelerationBufferIncrement,
					std::string & errorInfo)
{
	errorInfo.clear();

	if(index >= STEPPER_AMOUNT) {
		errorInfo = "stepper index is out of range: " + std::to_string(index);
	}
	if((accelerationBufferDecrement == 0) || (decelerationBufferIncrement == 0)) {
		errorInfo = "invalid parameter value";
	}
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement " + errorInfo);
		return false;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	_consoleCommand.stepperIndex = index;
	_consoleCommand.lowClks = lowClks;
	_consoleCommand.highClks = highClks;
	_consoleCommand.accelerationBuffer = accelerationBuffer;
	_consoleCommand.accelerationBufferDecrement = accelerationBufferDecrement;
	_consoleCommand.decelerationBuffer = decelerationBuffer;
	_consoleCommand.decelerationBufferIncrement = decelerationBufferIncrement;

	std::string command = ConsoleCommandFactory::CmdStepperConfigStep(index, lowClks, highClks);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement failed to configure steps : " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdStepperAccelerationBuffer(index, accelerationBuffer);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement failed to configure accelerationBuffer : " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdStepperAccelerationBufferDecrement(index, accelerationBufferDecrement);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement failed to configure accelerationBufferDecrement : " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdStepperDecelerationBuffer(index, decelerationBuffer);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement failed to configure decelerationBuffer : " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdStepperDecelerationBufferIncrement(index, decelerationBufferIncrement);
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement failed to configure decelerationBufferIncrement : " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::StepperConfigForwardClockwise(unsigned int index, bool forwardClockwise, std::string & errorInfo)
{
	errorInfo.clear();

	if(index >= STEPPER_AMOUNT) {
		errorInfo = "stepper index is out of range: " + std::to_string(index);
	}
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::StepperConfigMovement " + errorInfo);
		return false;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	//not implemented in webserver yet.

	return false;
}

bool WebServer::BdcForward(unsigned int index, std::string & errorInfo)
{
	unsigned long lowClks, highClks, cycles;
	std::string command;

	errorInfo.clear();
	if(index >= BDC_AMOUNT) {
		errorInfo = "bdc index is out of range, index:" + std::to_string(index);
		pLogger->LogError("WebServer::BdcForward " + errorInfo);
		return false;
	}

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	command = ConsoleCommandFactory::CmdBdcForward(index, lowClks, highClks, cycles);

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	_consoleCommand.bdcIndex = index;
	_consoleCommand.bdcStatus = BdcStatus::FORWARD;
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::BdcForward failed to forward bdc: " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::BdcReverse(unsigned int index, std::string & errorInfo)
{
	unsigned long lowClks, highClks, cycles;
	std::string command;

	errorInfo.clear();
	if(index >= BDC_AMOUNT) {
		errorInfo = "bdc index is out of range, index:" + std::to_string(index);
		pLogger->LogError("WebServer::BdcReverse " + errorInfo);
		return false;
	}

	pMovementConfiguration->GetBdcConfig(lowClks, highClks, cycles);
	command = ConsoleCommandFactory::CmdBdcReverse(index, lowClks, highClks, cycles);

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	_consoleCommand.bdcIndex = index;
	_consoleCommand.bdcStatus = BdcStatus::REVERSE;
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::BdcReverse failed to reverse bdc: " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::BdcDeactivate(unsigned int index, std::string & errorInfo)
{
	std::string command;

	errorInfo.clear();
	if(index >= BDC_AMOUNT) {
		errorInfo = "bdc index is out of range, index:" + std::to_string(index);
		pLogger->LogError("WebServer::BdcDeactivate " + errorInfo);
		return false;
	}

	command = ConsoleCommandFactory::CmdBdcBreak(index);

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	_consoleCommand.bdcIndex = index;
	_consoleCommand.bdcStatus = BdcStatus::BREAK;
	runConsoleCommand(command, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::BdcDeactivate failed to deactivate bdc: " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::PowerOn(const std::string& target, bool on, std::string & errorInfo)
{
	errorInfo.clear();
	std::string cmd;

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	if(target == "steppers") {
		if(on) {
			cmd = ConsoleCommandFactory::CmdSteppersPowerOn();
		}
		else {
			cmd = ConsoleCommandFactory::CmdSteppersPowerOff();
		}

		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::PowerOn failed to set stepper power: " + errorInfo);
			return false;
		}
	}
	else if(target == "bdcs") {
		if(on) {
			cmd = ConsoleCommandFactory::CmdBdcsPowerOn();
		}
		else {
			cmd = ConsoleCommandFactory::CmdBdcsPowerOff();
		}

		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::PowerOn failed to set bdc power: " + errorInfo);
			return false;
		}
	}
	else if(target == "opt") {
		if(on) {
			cmd = ConsoleCommandFactory::CmdOptPowerOn();
		}
		else {
			cmd = ConsoleCommandFactory::CmdOptPowerOff();
		}

		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::PowerOn failed to set opt power: " + errorInfo);
			return false;
		}
	}
//	else if(target == "dcm0") {
//
//	}
//	else if(target == "dcm1") {
//
//	}
	else
	{
		errorInfo = "wrong target: " + target;
		pLogger->LogError("WebServer::PowerOn " + errorInfo);
		return false;
	}

	return true;
}

bool WebServer::Query(std::string & errorInfo)
{
	errorInfo.clear();
	std::string cmd;

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex); //one command at a time

	cmd = ConsoleCommandFactory::CmdDeviceQueryPower();
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::Query failed in device query power: " + errorInfo);
		return false;
	}

	cmd = ConsoleCommandFactory::CmdDeviceQueryFuse();
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::Query failed in device query fuse: " + errorInfo);
		return false;
	}

//	cmd = ConsoleCommandFactory::CmdOptQueryPower();
//	runConsoleCommand(cmd, errorInfo);
//	if(!errorInfo.empty()) {
//		pLogger->LogError("WebServer::Query failed in opt query power: " + errorInfo);
//		return false;
//	}

	cmd = ConsoleCommandFactory::CmdBdcsQueryPower();
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::Query failed in bdc query power: " + errorInfo);
		return false;
	}

//	cmd = ConsoleCommandFactory::CmdDcmQueryPower();
//	runConsoleCommand(cmd, errorInfo);
//	if(!errorInfo.empty()) {
//		pLogger->LogError("WebServer::Query failed in dcm query power: " + errorInfo);
//		return false;
//	}

	cmd = ConsoleCommandFactory::CmdSteppersQueryPower();
	runConsoleCommand(cmd, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::Query failed in stepper query power: " + errorInfo);
		return false;
	}

	for(unsigned int i=0; i<LOCATOR_AMOUNT; i++)
	{
		_consoleCommand.locatorIndex = i;
		cmd = ConsoleCommandFactory::CmdLocatorQuery(i);
		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::Query failed in locator query: " + std::to_string(i) + "; error:" + errorInfo);
			return false;
		}
	}

	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		_consoleCommand.stepperIndex = i;
		cmd = ConsoleCommandFactory::CmdStepperQuery(i);
		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::Query failed in stepper query: " + std::to_string(i) + "; error:" + errorInfo);
			return false;
		}
	}

	for(unsigned int i=0; i<BDC_AMOUNT; i++)
	{
		_consoleCommand.bdcIndex = i;
		cmd = ConsoleCommandFactory::CmdBdcQuery(i);
		runConsoleCommand(cmd, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::Query failed in bdc query: " + std::to_string(i) + "; error:" + errorInfo);
			return false;
		}
	}

	return true;
}

bool WebServer::SaveCoordinate(const std::string & coordinateType, unsigned int data, std::string & errorInfo)
{
	errorInfo.clear();

	if(coordinateType == "smartCard")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SmartCard,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of smart card: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}

		rc = pCoordinateStorage->SetSmartCardOffset(data, _consoleCommand.resultSteppers[4].homeOffset);
		if(rc == false) {
			errorInfo = "failed to save offset of smart card: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "smartCardGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SmartCardGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of smart card gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "pedKey")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::PedKey,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of ped key: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "pedKeyPressed")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::PedKeyPressed,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of ped key pressed: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "pedKeyGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::PedKeyGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of ped key gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "softKey")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SoftKey,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of soft key: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "softKeyPressed")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SoftKeyPressed,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of soft key pressed: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "softKeyGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SoftKeyGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of soft key gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "assistKey")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::AssistKey,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of assist key: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "assistKeyPressed")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::AssistKeyPressed,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of assist key pressed: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "assistKeyGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::AssistKeyGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of assist key gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "touchScreenKey")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::TouchScreenKey,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of touch screen key: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "touchScreenKeyPressed")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of touch screen key pressed: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "touchScreenKeyGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of touch screen key gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "smartCardReader")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SmartCardReader,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of smart card reader";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "smartCardReaderGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::SmartCardReaderGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of smart card reader gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "barcodeReader")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::BarCodeReader,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of bar code reader";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "barCodeReaderExtra")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::BarCodeReaderExtraPosition,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of bar code reader extra position: " + std::to_string(data);
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "barcodeReaderGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::BarCodeReaderGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of bar code reader gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "contactlessReader")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::ContactlessReader,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of contactless reader";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "contactlessReaderGate")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::ContactlessReaderGate,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of contactless reader gate";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "safe")
	{
		auto rc = pCoordinateStorage->SetCoordinate(CoordinateStorage::Type::Safe,
				_consoleCommand.resultSteppers[0].homeOffset,
				_consoleCommand.resultSteppers[1].homeOffset,
				_consoleCommand.resultSteppers[2].homeOffset,
				_consoleCommand.resultSteppers[3].homeOffset,
				data);

		if(rc == false) {
			errorInfo = "failed to save coordinate of safe";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
		}
	}
	else if(coordinateType == "smartCardSlowlyPlaceStartZ") {
		pCoordinateStorage->SetSmartCardSlowlyPlaceStartZ(data);
	}
	else if(coordinateType == "smartCardSlowlyPlaceEndZ") {
		pCoordinateStorage->SetSmartCardSlowlyPlaceEndZ(data);
	}
	else if(coordinateType == "smartCardFetchOffset") {
		pCoordinateStorage->SetSmartCardFetchOffset(data);
	}
	else if(coordinateType == "smartCardReleaseOffsetZ") {
		pCoordinateStorage->SetSmartCardReleaseOffsetZ(data);
	}
	else if(coordinateType == "smartCardInsertExtra") {
		pCoordinateStorage->SetSmartCardInsertExtra(data);
	}
	else if(coordinateType == "smartCardReaderSlowInsertEndY") {
		pCoordinateStorage->SetSmartCardReaderSlowInsertEndY(data);
	}
	else if(coordinateType == "maximum")
	{
		switch(data)
		{
			case 0:
				pCoordinateStorage->SetMaximumX(_consoleCommand.resultSteppers[STEPPER_X].homeOffset);
				break;

			case 1:
				pCoordinateStorage->SetMaximumY(_consoleCommand.resultSteppers[STEPPER_Y].homeOffset);
				break;

			case 2:
				pCoordinateStorage->SetMaximumZ(_consoleCommand.resultSteppers[STEPPER_Z].homeOffset);
				break;

			case 3:
				pCoordinateStorage->SetMaximumW(_consoleCommand.resultSteppers[STEPPER_W].homeOffset);
				break;

			default:
				errorInfo = "unknown stepper index: " + std::to_string(data);
				pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
				break;
		}
	}
	else {
		errorInfo = "unknown coordinate type: " + coordinateType;
		pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
	}

	if(errorInfo.empty())
	{
		if(pCoordinateStorage->PersistToFile()) {
			return true;
		}
		else {
			errorInfo = "failed to persist coordinate to file";
			pLogger->LogError("WebServer::SaveCoordinate " + errorInfo);
			return false;
		}
	}
	else {
		return false;
	}
}

bool WebServer::ToCoordinate(const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int w, std::string & errorInfo)
{
	errorInfo.clear();
	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex);

	unsigned int curX = _consoleCommand.resultSteppers[STEPPER_X].homeOffset;
	unsigned int curY = _consoleCommand.resultSteppers[STEPPER_Y].homeOffset;
	unsigned int curZ = _consoleCommand.resultSteppers[STEPPER_Z].homeOffset;
	unsigned int curW = _consoleCommand.resultSteppers[STEPPER_W].homeOffset;

	if(curZ < z)
	{
		//move up, starting from Z
		StepperMove(STEPPER_Z, true, z-curZ, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Z: " + errorInfo);
			return false;
		}

		//move W
		bool wForward = (w > curW);
		unsigned int wSteps = wForward?(w-curW):(curW-w);
		StepperMove(STEPPER_W, wForward, wSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper W: " + errorInfo);
			return false;
		}

		//move Y
		bool yForward = (y > curY);
		unsigned int ySteps = yForward?(y-curY):(curY-y);
		StepperMove(STEPPER_Y, yForward, ySteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}

		//move X
		bool xForward = (x > curX);
		unsigned int xSteps = xForward?(x-curX):(curX-x);
		StepperMove(STEPPER_X, xForward, xSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper X: " + errorInfo);
			return false;
		}
	}
	else
	{
		//move down, starting from Y
		bool yForward = (y > curY);
		unsigned int ySteps = yForward?(y-curY):(curY-y);
		StepperMove(STEPPER_Y, yForward, ySteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}

		//move W
		bool wForward = (w > curW);
		unsigned int wSteps = wForward?(w-curW):(curW-w);
		StepperMove(STEPPER_W, wForward, wSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper W: " + errorInfo);
			return false;
		}

		//move Z
		StepperMove(STEPPER_Z, false, curZ-z, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Z: " + errorInfo);
			return false;
		}

		//move X
		bool xForward = (x > curX);
		unsigned int xSteps = xForward?(x-curX):(curX-x);
		StepperMove(STEPPER_X, xForward, xSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper X: " + errorInfo);
			return false;
		}
	}

	return true;
}

bool WebServer::ToCoordinateItem(const unsigned int index, unsigned int coordinate, std::string & errorInfo)
{
	errorInfo.clear();
	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex);

	if(index >= STEPPER_AMOUNT) {
		errorInfo = "stepper index out of range: " + std::to_string(index);
		pLogger->LogError("WebServer::ToCoordinateItem stepper index out of range: " + std::to_string(index));
		return false;
	}

	unsigned int curOffset = _consoleCommand.resultSteppers[index].homeOffset;

	bool forward = (coordinate > curOffset);
	unsigned int steps = forward?(coordinate - curOffset):(curOffset - coordinate);

	StepperMove(index, forward, steps, errorInfo);
	if(!errorInfo.empty()) {
		pLogger->LogError("WebServer::ToCoordinateItem failed to move stepper: " + std::to_string(index));
		return false;
	}

	return true;
}

bool WebServer::ToCoordinateIndirect(const unsigned int x, const unsigned int y, const unsigned int z, const unsigned int w, std::string & errorInfo)
{
	errorInfo.clear();

	long maxY;
	if(!pCoordinateStorage->GetMaximumY(maxY))
	{
		errorInfo = "failed to retrieve maximum Y";
		pLogger->LogError("WebServer::ToCoordinateIndirect " + errorInfo);
		return false;
	}

	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex);

	unsigned int curX = _consoleCommand.resultSteppers[STEPPER_X].homeOffset;
	unsigned int curY = _consoleCommand.resultSteppers[STEPPER_Y].homeOffset;
	unsigned int curZ = _consoleCommand.resultSteppers[STEPPER_Z].homeOffset;
	unsigned int curW = _consoleCommand.resultSteppers[STEPPER_W].homeOffset;

	if(curZ < z)
	{
		//move up, starting from Z
		StepperMove(STEPPER_Z, true, z-curZ, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Z: " + errorInfo);
			return false;
		}

		//move Y
		StepperMove(STEPPER_Y, true, maxY-curY, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}

		//move W
		bool wForward = (w > curW);
		unsigned int wSteps = wForward?(w-curW):(curW-w);
		StepperMove(STEPPER_W, wForward, wSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper W: " + errorInfo);
			return false;
		}

		//move X
		bool xForward = (x > curX);
		unsigned int xSteps = xForward?(x-curX):(curX-x);
		StepperMove(STEPPER_X, xForward, xSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper X: " + errorInfo);
			return false;
		}

		//move Y
		StepperMove(STEPPER_Y, false, maxY-y, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}
	}
	else
	{
		//move down, starting from Y
		StepperMove(STEPPER_Y, true, maxY-curY, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}

		//move W
		bool wForward = (w > curW);
		unsigned int wSteps = wForward?(w-curW):(curW-w);
		StepperMove(STEPPER_W, wForward, wSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper W: " + errorInfo);
			return false;
		}

		//move Z
		StepperMove(STEPPER_Z, false, curZ-z, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Z: " + errorInfo);
			return false;
		}

		//move X
		bool xForward = (x > curX);
		unsigned int xSteps = xForward?(x-curX):(curX-x);
		StepperMove(STEPPER_X, xForward, xSteps, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper X: " + errorInfo);
			return false;
		}

		StepperMove(STEPPER_Y, false, maxY-y, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToCoordinate fail to move stepper Y: " + errorInfo);
			return false;
		}
	}

	return true;
}

bool WebServer::ToSmartCardOffset(const unsigned int offset, std::string & errorInfo)
{
	errorInfo.clear();
	Poco::ScopedLock<Poco::Mutex> lock(_webServerMutex);

	unsigned int curV = _consoleCommand.resultSteppers[STEPPER_V].homeOffset;
	if(offset > curV) {
		StepperMove(STEPPER_V, true, offset - curV, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToSmartCardOffset fail to move stepper V: " + errorInfo);
			return false;
		}
	}
	else {
		StepperMove(STEPPER_V, false, offset - curV, errorInfo);
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::ToSmartCardOffset fail to move stepper V: " + errorInfo);
			return false;
		}
	}

	return true;
}

std::string WebServer::DeviceStatus()
{
	std::string json;

	json += "{";
	//steppers
	for(unsigned int i=0; i<STEPPER_AMOUNT; i++)
	{
		//stepper i object
		auto& data = _consoleCommand.resultSteppers[i];

		json += "\"stepper" + std::to_string(i) + "\":{";
			json += "\"state\":" + std::to_string((int)(data.state)) + ",";
			json += "\"enabled\":" + (data.enabled?std::string("true"):std::string("false")) + ",";
			json += "\"forward\":" + (data.forward?std::string("true"):std::string("false")) + ",";
			json += "\"forwardClockwise\":" + (data.forwardClockwise?std::string("true"):std::string("false")) + ",";
			json += "\"homeOffset\":" + std::to_string(data.homeOffset) + ",";
			json += "\"locatorIndex\":" + std::to_string(data.locatorIndex) + ",";
			json += "\"locatorLineNumberStart\":" + std::to_string(data.locatorLineNumberStart) + ",";
			json += "\"locatorLineNumberTerminal\":" + std::to_string(data.locatorLineNumberTerminal) + ",";
			json += "\"maximum\":" + std::to_string(data.maximum) + ",";
			json += "\"lowClks\":" + std::to_string(data.lowClks) + ",";
			json += "\"highClks\":" + std::to_string(data.highClks) + ",";
			json += "\"accelerationBuffer\":" + std::to_string(data.accelerationBuffer) + ",";
			json += "\"accelerationBufferDecrement\":" + std::to_string(data.accelerationBufferDecrement) + ",";
			json += "\"decelerationBuffer\":" + std::to_string(data.decelerationBuffer) + ",";
			json += "\"decelerationBufferIncrement\":" + std::to_string(data.decelerationBufferIncrement);
		json += "},";
	}
	//locators
	for(unsigned int i=0; i<LOCATOR_AMOUNT; i++)
	{
		//locator i
		json += "\"locator" + std::to_string(i) + "\":" + std::to_string(_consoleCommand.resultLocators[i]) + ",";
	}
	//bdcs
	for(unsigned int i=0; i<BDC_AMOUNT; i++)
	{
		unsigned int action;

		switch(_consoleCommand.resultBdcStatus[i])
		{
		case BdcStatus::FORWARD:
			action = 0;
			break;

		case BdcStatus::REVERSE:
			action = 1;
			break;

		default:
			action = 2;
			break;
		}
		//bdc i
		json += "\"bdc" + std::to_string(i) + "\":" + std::to_string(action) + ",";
	}
	//offsets
	{
		long smartCardSlowlyPlaceStartZ = 0;
		long smartCardSlowlyPlaceEndZ = 0;
		long smartCardFetchOffset = 0;
		long smartCardReleaseOffset = 0;
		long smartCardInsertExtra = 0;
		long smartCardReaderSlowInsertEndY = 0;

		if(!pCoordinateStorage->GetSmartCardSlowlyPlaceStartZ(smartCardSlowlyPlaceStartZ)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardPlaceStartZ");
		}
		if(!pCoordinateStorage->GetSmartCardSlowlyPlaceEndZ(smartCardSlowlyPlaceEndZ)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardPlaceEndZ");
		}
		if(!pCoordinateStorage->GetSmartCardFetchOffset(smartCardFetchOffset)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardFetchOffset");
		}
		if(!pCoordinateStorage->GetSmartCardReleaseOffset(smartCardReleaseOffset)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardReleaseOffset");
		}
		if(!pCoordinateStorage->GetSmartCardInsertExtra(smartCardInsertExtra)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardInsertExtra");
		}
		if(!pCoordinateStorage->GetSmartCardReaderSlowInsertEndY(smartCardReaderSlowInsertEndY)) {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smartCardReaderSlowInsertEndY");
		}

		json += "\"smartCardSlowlyPlaceStartZ\":" + std::to_string(smartCardSlowlyPlaceStartZ) + ",";
		json += "\"smartCardSlowlyPlaceEndZ\":" + std::to_string(smartCardSlowlyPlaceEndZ) + ",";
		json += "\"smartCardFetchOffset\":" + std::to_string(smartCardFetchOffset) + ",";
		json += "\"smartCardReleaseOffset\":" + std::to_string(smartCardReleaseOffset) + ",";
		json += "\"smartCardInsertExtra\":" + std::to_string(smartCardInsertExtra) + ",";
		json += "\"smartCardReaderSlowInsertEndY\":" + std::to_string(smartCardReaderSlowInsertEndY) + ",";

	}
	//others
	json += "\"deviceConnected\":" + std::string(_consoleCommand.resultDeviceConnected?"true":"false") + ",";
	json += "\"devicePowered\":" + std::string(_consoleCommand.resultDevicePowered?"true":"false") + ",";
	json += "\"deviceFuseOk\":" + std::string(_consoleCommand.resultDeviceFuseOk?"true":"false") + ",";
	json += "\"optPowered\":" + std::string(_consoleCommand.resultOptPowered?"true":"false") + ",";
	json += "\"bdcPowered\":" + std::string(_consoleCommand.resultBdcsPowered?"true":"false") + ",";
	json += "\"stepperPowered\":" + std::string(_consoleCommand.resultSteppersPowered?"true":"false") + ",";

	//coordinateSmartCardGate
	json += "\"coordinateSmartCardGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w) ;
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of smart card gate");
		}
	}
	json += "},";
	//coordinateSmartCards
	json += "\"coordinateSmartCards\":[";
	for(unsigned int i=0; i<pCoordinateStorage->SmartCardsAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCard, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of smart card: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->SmartCardsAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//coordinate Smart card offsets
	json += "\"coordinateSmartCardOffsets\":[";
	for(unsigned int i=0; i<pCoordinateStorage->SmartCardsAmount(); i++)
	{
		int value = 0;

		json += "{";
		if(pCoordinateStorage->GetSmartCardOffset(i, value)) {
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve smart card offset: " + std::to_string(i));
		}
		json += "\"index\":" + std::to_string(i) + ",";
		json += "\"value\":" + std::to_string(value) ;
		json += "},";
	}
	if(pCoordinateStorage->SmartCardsAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//pedKeyGate
	json += "\"coordinatePedKeyGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of ped key gate");
		}
	}
	json += "},";
	//pedKeys
	json += "\"coordinatePedKeys\":[";
	for(unsigned int i=0; i<pCoordinateStorage->PedKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKey, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of ped key: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->PedKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//pedKeysPressed
	json += "\"coordinatePedKeysPressed\":[";
	for(unsigned int i=0; i<pCoordinateStorage->PedKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::PedKeyPressed, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of ped key pressed: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->PedKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//softKeyGate
	json += "\"coordinateSoftKeyGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of soft key gate");
		}
	}
	json += "},";
	//softKeys
	json += "\"coordinateSoftKeys\":[";
	for(unsigned int i=0; i<pCoordinateStorage->SoftKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKey, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of soft key: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->SoftKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//softKeysPressed
	json += "\"coordinateSoftKeysPressed\":[";
	for(unsigned int i=0; i<pCoordinateStorage->SoftKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SoftKeyPressed, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of soft key pressed: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->SoftKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//assistKeyGate
	json += "\"coordinateAssistKeyGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of assist key gate");
		}
	}
	json += "},";
	//assistKeys
	json += "\"coordinateAssistKeys\":[";
	for(unsigned int i=0; i<pCoordinateStorage->AssistKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKey, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of assist key: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->AssistKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//assistKeysPressed
	json += "\"coordinateAssistKeysPressed\":[";
	for(unsigned int i=0; i<pCoordinateStorage->AssistKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::AssistKeyPressed, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of assist key pressed: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->AssistKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//touchScreenKeyGate
	json += "\"coordinateTouchScreenKeyGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of touch screen key gate");
		}
	}
	json += "},";
	//touchScreenKeys
	json += "\"coordinateTouchScreenKeys\":[";
	for(unsigned int i=0; i<pCoordinateStorage->TouchScreenKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKey, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of touch screen key: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->TouchScreenKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//touchScreenKeysPressed
	json += "\"coordinateTouchScreenKeysPressed\":[";
	for(unsigned int i=0; i<pCoordinateStorage->TouchScreenKeysAmount(); i++)
	{
		int x, y, z, w;

		json += "{";
		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::TouchScreenKeyPressed, x, y, z, w, i)) {
			json += "\"index\":" + std::to_string(i) + ",";
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of touch screen key pressed: " + std::to_string(i));
		}
		json += "},";
	}
	if(pCoordinateStorage->TouchScreenKeysAmount()) {
		json.pop_back();//remove the last ','
	}
	json += "],";
	//smartCardReaderGate
	json += "\"coordinateSmartCardReaderGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReaderGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w) ;
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of smart card reader gate");
		}
	}
	json += "},";
	//smartCardReader
	json += "\"coordinateSmartCardReader\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::SmartCardReader, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of smart card reader");
		}
	}
	json += "},";
	//barCodeReaderGate
	json += "\"coordinateBarCodeReaderGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReaderGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of bar code reader gate");
		}
	}
	json += "},";
	//barCodeReader
	json += "\"coordinateBarCodeReader\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::BarCodeReader, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of bar code reader");
		}
	}
	json += "},";
	//contactlessReaderGate
	json += "\"coordinateContactlessReaderGate\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReaderGate, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of contactless reader gate");
		}
	}
	json += "},";
	//contactlessReader
	json += "\"coordinateContactlessReader\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::ContactlessReader, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of contactless reader");
		}
	}
	json += "},";
	//safe
	json += "\"coordinateSafe\":{";
	{
		int x, y, z, w;

		if(pCoordinateStorage->GetCoordinate(CoordinateStorage::Type::Safe, x, y, z, w, 0)) {
			json += "\"x\":" + std::to_string(x) + ",";
			json += "\"y\":" + std::to_string(y) + ",";
			json += "\"z\":" + std::to_string(z) + ",";
			json += "\"w\":" + std::to_string(w);
		}
		else {
			pLogger->LogError("WebServer::DeviceStatus failed to retrieve coordinate of safe");
		}
	}
	json += "}";

	//end of json
	json += "}";

	pLogger->LogInfo("WebServer::DeviceStatus content: " + json);
	return json;
}

void WebServer::runConsoleCommand(const std::string & cmd, std::string & errorInfo)
{
	std::string cmdToLog;

	//log command content except \r\n
	for(auto it=cmd.begin(); it!=cmd.end(); it++) {
		char c = *it;
		if((c >= ' ') && (c <= '~')) {
			cmdToLog.push_back(c);
		}
	}

	// start command
	{
		Poco::ScopedLock<Poco::Mutex> lock(_replyMutex); //synchronize console command and reply

		if(_consoleCommand.state != CommandState::Idle) {
			errorInfo = "internal error: wrong command state";
		}
		if(!errorInfo.empty()) {
			pLogger->LogError("WebServer::runConsoleCommand " + errorInfo);
			return;
		}

		_consoleCommand.state = CommandState::OnGoing;
		_consoleCommand.cmdId = _pConsoleOperator->RunConsoleCommand(cmd);
		pLogger->LogInfo("WebServer::runConsoleCommand command id: " + std::to_string(_consoleCommand.cmdId));
		if(_consoleCommand.cmdId == InvalidCommandId)
		{
			errorInfo = "failure in running command";
			pLogger->LogError("WebServer::runConsoleCommand failed to start console command: " + cmdToLog);
			_consoleCommand.state = CommandState::Idle;
			return;
		}
	}

	//wait for result
	for(;;)
	{
		CommandState tmpState;
		{
			Poco::ScopedLock<Poco::Mutex> lock(_replyMutex);
			tmpState = _consoleCommand.state;
		}

		if(tmpState == CommandState::OnGoing) {
			sleep(10);
		}
		else {
			break;
		}
	}

	//check result
	if(_consoleCommand.state == CommandState::Succeeded) {
		pLogger->LogInfo("WebServer::runConsoleCommand succeeded in: " + cmdToLog);
	}
	else if(_consoleCommand.state == CommandState::Failed) {
		errorInfo = "failed in running command";
		pLogger->LogError("WebServer::runConsoleCommand failed to run console command: " + cmdToLog);
	}
	else {
		errorInfo = "failed in running command";
		pLogger->LogError("WebServer::runConsoleCommand wrong command result: " + std::to_string((int)_consoleCommand.state));
	}

	_consoleCommand.cmdId = InvalidCommandId;
	_consoleCommand.state = CommandState::Idle;
}

void WebServer::runTask()
{
	Poco::ThreadPool::defaultPool().addCapacity(_maxThread);

	Poco::Net::HTTPServerParams* pParams = new Poco::Net::HTTPServerParams;
	pParams->setMaxQueued(_maxQueue);
	pParams->setMaxThreads(_maxThread);

	pLogger->LogInfo("WebServer::runTask starts");

	try
	{
		// set-up a server socket
		Poco::Net::ServerSocket svs(_port);
		// set-up a HTTPServer instance
		Poco::Net::HTTPServer srv(new ScsRequestHandlerFactory(this), svs, pParams);
		// start the HTTPServer
		srv.start();
		// wait for CTRL-C or kill
		while(1)
		{
			if(isCancelled()) {
				break;
			}
			sleep(10);
		}
		// Stop the HTTPServer
		srv.stop();
	}
	catch(Poco::Exception &e)
	{
		pLogger->LogError("WebServer::runTask exception happened: " + e.displayText());
	}
	catch(...)
	{
		pLogger->LogError("WebServer::runTask unknown exception happened");
	}

	pLogger->LogInfo("WebServer::runTask exists");
}
