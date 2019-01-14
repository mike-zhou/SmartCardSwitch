/*
 * WebServer.h
 *
 *  Created on: Jan 3, 2019
 *      Author: mikez
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <string>
#include <vector>
#include <deque>

#include "Poco/Task.h"
#include "Poco/Event.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"

#include "CoordinateStorage.h"
#include "ConsoleCommandFactory.h"
#include "ICommandReception.h"
#include "ConsoleOperator.h"

class WebServer;

class ScsRequestHandler: public Poco::Net::HTTPRequestHandler
	/// Return a HTML document with the current date and time.
{
public:
	ScsRequestHandler(WebServer * pWebServer) { _pWebServer = pWebServer;}

private:
	//Poco::Net::HTTPRequestHandler
	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

private:
	WebServer * _pWebServer;

	std::string getJsonCommand(Poco::Net::HTTPServerRequest& request);

	//***************************
	//command handlers
	//***************************
	//
	//request:
	//	uri: /
	//	body: empty
	void onDefaultHtml(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	//request:
	//	uri: /stepper
	//	body:
	//	 {
	//		"index":0,
	//		"forward":true,
	//		"steps":1
	//	 }
	void onStepperMove(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	//request:
	//	uri: /query
	//	body: empty
	void onQuery(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	//request:
	//	uri: /bdc
	//	body:
	//	 {
	//		"index":0,
	//		"action":0 //0: forward; 1: reverse; 2: deactivate
	//	 }
	void onBdc(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	//request:
	//	uri: /saveCoordinate
	//	body:
	//		{
	//			"coordinateType":"smartCard",
	//			"index":0
	//		}
	void onSaveCoordinate(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	//request:
	//	uri: /stepperConfigMovement
	// 	body:
	//	{
	//		"index":1,
	//		"lowClks":1,
	//		"highClks":1,
	//		"accelerationBuffer":1,
	//		"accelerationBufferDecrement":1,
	//		"decelerationBuffer":1,
	//		"decelerationBufferIncrement":1
	//	}
	void onStepperConfigMovement(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

	void onStepperConfigHome(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
};


class ScsRequestHandlerFactory: public Poco::Net::HTTPRequestHandlerFactory
{
public:
	ScsRequestHandlerFactory(WebServer * pWebServer) { _pWebServer = pWebServer; }

	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		return new ScsRequestHandler(_pWebServer);
	}

private:
	WebServer * _pWebServer;
};


class WebServer: public Poco::Task, public IResponseReceiver
{
public:
	WebServer(unsigned int port, unsigned int maxQueue, unsigned int maxThread, const std::string & filesFolder);

	void SetConsoleOperator(ConsoleOperator * pCO) { _pConsoleOperator = pCO; }

	/**
	 * APIs for requests
	 */
	//returns the default web page.
	const std::string& GetDefaultPageContent();
	//move stepper. return true with empty errorInfo if succeed, false with detailed error if fail.
	bool StepperMove(unsigned int index, bool forward, unsigned int steps, std::string & errorInfo);
	bool StepperConfigBoundary(
						unsigned int stepperIndex,
						unsigned int locatorIndex,
						unsigned int locatorLineNumberStart,
						unsigned int locatorLineNumberTerminal,
						std::string & errorInfo);
	bool StepperConfigMovement(
						unsigned int index,
						unsigned int lowClks,
						unsigned int highClks,
						unsigned int accelerationBuffer,
						unsigned int accelerationBufferDecrement,
						unsigned int decelerationBuffer,
						unsigned int decelerationBufferIncrement,
						std::string & errorInfo);
	bool BdcForward(unsigned int index, std::string & errorInfo);
	bool BdcReverse(unsigned int index, std::string & errorInfo);
	bool BdcDeactivate(unsigned int index, std::string & errorInfo);
	bool OptPowerOn(std::string & errorInfo);
	bool OptPowerOff(std::string & errorInfo);
	bool Query(std::string & errorInfo);
	bool SaveCoordinate(const std::string & coordinateType, unsigned int data, std::string & errorInfo);
	//return a JSON string representing current device status.
	std::string DeviceStatus();

private:
	//Poco::Task
	void runTask() override;

	//IResponseReceiver
	virtual void OnDevicesGet(CommandId key, bool bSuccess, const std::vector<std::string>& devices) override;
	virtual void OnDeviceConnect(CommandId key, bool bSuccess) override;
	virtual void OnDeviceQueryPower(CommandId key, bool bSuccess, bool bPowered)  override;
	virtual void OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn) override;
	virtual void OnDeviceDelay(CommandId key, bool bSuccess)  override;
 	virtual void OnOptPowerOn(CommandId key, bool bSuccess)  override;
	virtual void OnOptPowerOff(CommandId key, bool bSuccess)   override;
	virtual void OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnDcmPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnDcmPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnBdcsPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnBdcsPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnBdcCoast(CommandId key, bool bSuccess) override;
	virtual void OnBdcReverse(CommandId key, bool bSuccess) override;
	virtual void OnBdcForward(CommandId key, bool bSuccess) override;
	virtual void OnBdcBreak(CommandId key, bool bSuccess) override;
	virtual void OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status) override;
	virtual void OnSteppersPowerOn(CommandId key, bool bSuccess) override;
	virtual void OnSteppersPowerOff(CommandId key, bool bSuccess) override;
	virtual void OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered) override;
	virtual void OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs) override;
	virtual void OnStepperConfigStep(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBuffer(CommandId key, bool bSuccess) override;
	virtual void OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess) override;
	virtual void OnStepperEnable(CommandId key, bool bSuccess) override;
	virtual void OnStepperForward(CommandId key, bool bSuccess) override;
	virtual void OnStepperSteps(CommandId key, bool bSuccess) override;
	virtual void OnStepperRun(CommandId key, bool bSuccess) override;
	virtual void OnStepperConfigHome(CommandId key, bool bSuccess) override;
	virtual void OnStepperMove(CommandId key, bool bSuccess) override;
	virtual void OnStepperQuery(CommandId key, bool bSuccess,
								StepperState state,
								bool bEnabled,
								bool bForward,
								unsigned int locatorIndex,
								unsigned int locatorLineNumberStart,
								unsigned int locatorLineNumberTerminal,
								unsigned long homeOffset,
								unsigned long lowClks,
								unsigned long highClks,
								unsigned long accelerationBuffer,
								unsigned long accelerationBufferDecrement,
								unsigned long decelerationBuffer,
								unsigned long decelerationBufferIncrement) override;

	virtual void OnStepperSetState(CommandId key, bool bSuccess) override;
	virtual void OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput) override;

private:
	static const int STEPPER_AMOUNT = 4;
	static const int LOCATOR_AMOUNT = 8;
	static const int BDC_AMOUNT = 6;

	Poco::Mutex _webServerMutex;
	Poco::Mutex _replyMutex;

	enum class CommandState
	{
		Idle = 0,
		OnGoing,
		Failed,
		Succeeded
	};

	struct ConsoleCommand
	{
		ICommandReception::CommandId cmdId;
		CommandState state;

		//params
		unsigned int stepperIndex;
		unsigned int steps;
		bool stepperForward;
		unsigned int lowClks, highClks, accelerationBuffer, accelerationBufferDecrement, decelerationBuffer, decelerationBufferIncrement;
		unsigned int locatorIndex;
		unsigned int bdcIndex;
		BdcStatus bdcStatus;

		//command results
		std::vector<std::string> resultDevices;
		bool resultDeviceConnected;
		bool resultDevicePowered;
		bool resultDeviceFuseOk;
		bool resultOptPowered;
		bool resultBdcsPowered;
		bool resultSteppersPowered;
		struct StepperStatus
		{
			StepperState state;
			bool enabled;
			bool forward;
			//boundary
			unsigned int homeOffset;
			unsigned int targetPosition;
			unsigned int locatorIndex;
			unsigned int locatorLineNumberStart;
			unsigned int locatorLineNumberTerminal;
			//movement
			unsigned int lowClks;
			unsigned int highClks;
			unsigned int accelerationBuffer;
			unsigned int accelerationBufferDecrement;
			unsigned int decelerationBuffer;
			unsigned int decelerationBufferIncrement;
			unsigned int maximum;
		} resultSteppers[STEPPER_AMOUNT];
		unsigned char resultLocators[LOCATOR_AMOUNT];
		BdcStatus resultBdcStatus[BDC_AMOUNT];
	};
	ConsoleCommand _consoleCommand;

	unsigned int _port;
	unsigned int _maxQueue;
	unsigned int _maxThread;
	std::string _filesFolder;

	std::string _defaultPageContent;

	ConsoleOperator * _pConsoleOperator;

	void runConsoleCommand(const std::string & cmd, std::string & errorInfo);
};

#endif /* WEBSERVER_H_ */
