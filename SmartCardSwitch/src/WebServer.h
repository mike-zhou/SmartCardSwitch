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

	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

private:
	WebServer * _pWebServer;

	void onDefaultHtml(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
	void onStepperMove(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
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

	const std::string& GetDefaultPageContent();
private:
	//Poco::Task
	void runTask();

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
	Poco::Mutex _mutex;

	enum class CommandState
	{
		Idle = 0,
		OnGoing,
		Failed,
		Succeeded
	} _cmdState;

	unsigned int _port;
	unsigned int _maxQueue;
	unsigned int _maxThread;
	std::string _filesFolder;

	std::string _defaultPageContent;

	ConsoleOperator * _pConsoleOperator;
};

#endif /* WEBSERVER_H_ */
