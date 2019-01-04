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

#include "WebServer.h"
#include "Logger.h"
#include "CoordinateStorage.h"
#include "MovementConfiguration.h"

extern Logger * pLogger;
extern CoordinateStorage * pCoordinateStorage;
extern MovementConfiguration * pMovementConfiguration;

void ScsRequestHandler::sendDefaultHtml(Poco::Net::HTTPServerResponse& response)
{
	auto defaultPage = _pWebServer->GetDefaultPageContent();

	response.setContentType("text/html");
	response.setChunkedTransferEncoding(true);

	std::ostream& ostr = response.send();
	ostr << defaultPage;
}

void ScsRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	pLogger->LogInfo("ScsRequestHandler::handleRequest %%%%%% URI: " + request.getURI());

	if(request.getURI() == "/") {
		sendDefaultHtml(response);
	}
	else if(request.getURI() == "/stepperMove") {

	}
	else {
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
	_cmdState = CommandState::Idle;
	_pConsoleOperator = nullptr;
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

			int fd = open(filePathName.c_str(), O_RDONLY);
			if(fd < 0) {
				pLogger->LogError("WebServer::GetDefaultPageContent cannot open file: " + filePathName);
			}
			else
			{
				for(;;)
				{
					unsigned char c;
					auto amount = read(fd, &c, 1);
					if(amount < 1) {
						break;
					}
					else {
						_defaultPageContent.push_back(c);
					}
				}
				//close file
				close(fd);
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

}

void WebServer::OnDeviceQueryFuse(CommandId key, bool bSuccess, bool bFuseOn)
{

}

void WebServer::OnDeviceDelay(CommandId key, bool bSuccess)
{

}

void WebServer::OnOptPowerOn(CommandId key, bool bSuccess)
{

}

void WebServer::OnOptPowerOff(CommandId key, bool bSuccess)
{

}

void WebServer::OnOptQueryPower(CommandId key, bool bSuccess, bool bPowered)
{

}

void WebServer::OnDcmPowerOn(CommandId key, bool bSuccess)
{

}

void WebServer::OnDcmPowerOff(CommandId key, bool bSuccess)
{

}

void WebServer::OnDcmQueryPower(CommandId key, bool bSuccess, bool bPowered)
{

}

void WebServer::OnBdcsPowerOn(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcsPowerOff(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcsQueryPower(CommandId key, bool bSuccess, bool bPowered)
{

}

void WebServer::OnBdcCoast(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcReverse(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcForward(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcBreak(CommandId key, bool bSuccess)
{

}

void WebServer::OnBdcQuery(CommandId key, bool bSuccess, BdcStatus status)
{

}

void WebServer::OnSteppersPowerOn(CommandId key, bool bSuccess)
{

}

void WebServer::OnSteppersPowerOff(CommandId key, bool bSuccess)
{

}

void WebServer::OnSteppersQueryPower(CommandId key, bool bSuccess, bool bPowered)
{

}

void WebServer::OnStepperQueryResolution(CommandId key, bool bSuccess, unsigned long resolutionUs)
{

}

void WebServer::OnStepperConfigStep(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperAccelerationBuffer(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperAccelerationBufferDecrement(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperDecelerationBuffer(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperDecelerationBufferIncrement(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperEnable(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperForward(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperSteps(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperRun(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperConfigHome(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperMove(CommandId key, bool bSuccess)
{

}

void WebServer::OnStepperQuery(CommandId key, bool bSuccess,
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
							unsigned long decelerationBufferIncrement)
{

}

void WebServer::OnStepperSetState(CommandId key, bool bSuccess)
{

}

void WebServer::OnLocatorQuery(CommandId key, bool bSuccess, unsigned int lowInput)
{

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
		Poco::Net::SocketAddress address("127.0.0.1:80");
		Poco::Net::ServerSocket svs(address);
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
