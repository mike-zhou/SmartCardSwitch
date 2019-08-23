//
// HTTPFormServer.cpp
//
// This sample demonstrates the HTTPServer and HTMLForm classes.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//

#include <iostream>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/PartHandler.h"
#include "Poco/Net/MessageHeader.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/CountingStream.h"
#include "Poco/NullStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Semaphore.h"
#include "Poco/TaskManager.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"

#include "Logger.h"

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Net::MessageHeader;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Poco::CountingInputStream;
using Poco::NullOutputStream;
using Poco::StreamCopier;

class PersistanceTask;

struct PendingFile
{
	enum PendingFileState
	{
		IDLE = 0,
		WRITING,
		READY,
		PERSISTING
	};

	PendingFileState state;
	std::string clientId;
	std::string fileName; //time stamp of frame creation in unit of millisecond
	unsigned char * pData;
	unsigned int actualSize;
	unsigned int maxSize;

	PendingFile()
	{
		state = IDLE;
		pData = NULL;
		maxSize = 0;
		actualSize = 0;
	}
};

struct PendingFileCache
{
	Poco::Mutex mutex;
	Poco::Semaphore * pFrameAvailableSemaphore;

	std::vector<PendingFile *> pendingFilePtrArray;
	std::deque<int> availableFrameIndexes;
};

static PendingFileCache * _pCache = NULL;
static std::string _frameRootFolder;
static std::vector<std::string> _clientIds;
static std::vector<PersistanceTask *> _persistanceTaskList;
static int _maxFramePeriods;

Logger * pLogger;

class PersistanceTask: public Poco::Task
{
public:
	PersistanceTask(const std::string & clientId) : Task(clientId)
	{
		frameFolder = _frameRootFolder;
		frameFolder.pushDirectory(clientId);
	}

	void AddPendingFileIndex(int index)
	{
		Poco::ScopedLock<Poco::Mutex> lock(mutex);
		pendingFrameIndexes.push_back(index);
		event.set(); //trigger the persistance
	}

private:
	Poco::Mutex mutex;
	std::deque<int> pendingFrameIndexes;
	Poco::Event event;
	Poco::Path frameFolder;
	Poco::Timestamp obsoleteFolderCheckTime;
	const long obsoleteFolderCheckInterval = 60000000; //1 minute

	virtual void runTask() override
	{
		int frameIndex;

		pLogger->LogInfo("PersistanceTask " + name() + "starts");

		for(;;)
		{
			if(isCancelled()) {
				break;
			}
			event.tryWait(1000); //try to wait for 1 seconds

			//persist pending frames
			for(;pendingFrameIndexes.empty() == false;)
			{
				{
					Poco::ScopedLock<Poco::Mutex> lock(mutex);
					frameIndex = pendingFrameIndexes[0];
					pendingFrameIndexes.pop_front();
				}

				auto pFrame = _pCache->pendingFilePtrArray[frameIndex];
				if(pFrame->state != PendingFile::PERSISTING) {
					//wrong frame state
					continue;
				}

				try
				{
					char buf[64];
					long minute = std::stol(pFrame->fileName)/60000;//change milliseconds to minutes
					FILE * pF;
					int count;

					Poco::Path folderPath = frameFolder;
					sprintf(buf, "%010ld", minute);
					folderPath.pushDirectory(buf);
					Poco::File folder(folderPath);
					if(folder.exists() == false) {
						folder.createDirectories();
					}

					Poco::Path filePath = folderPath;
					filePath.setFileName(pFrame->fileName + ".jpg");
					pF = fopen(filePath.toString().c_str(), "wb");
					if(pF == NULL) {
						pLogger->LogError("PersistanceTask failed to create: " + filePath.toString());
						throw Poco::Exception("failed to create file: " + filePath.toString());
					}
					count = fwrite(pFrame->pData, pFrame->actualSize, 1, pF);
					if(count != pFrame->actualSize) {
						pLogger->LogError("PersistanceTask partial persistence: " + std::to_string(count) + "/" + std::to_string(pFrame->actualSize));
					}
					fclose(pF);
				}
				catch(Poco::Exception & e)
				{
					pLogger->LogError("PersistanceTask exception: " + e.displayText());
				}
				catch(std::exception & e)
				{
					pLogger->LogError("PersistanceTask exception: " + std::string(e.what()));
				}
				catch(...)
				{
					pLogger->LogError("PersistanceTask unknown exception");
				}

				{
					Poco::ScopedLock<Poco::Mutex> lock(mutex);
					pFrame->state = PendingFile::IDLE;
				}
			}

			deleteObsoleteFiles();
		}

		pLogger->LogInfo("PersistanceTask " + name() + "exits");
	}

	void deleteObsoleteFiles()
	{
		if(obsoleteFolderCheckTime.elapsed() < obsoleteFolderCheckInterval)
			return;

		std::vector<std::string> folderNames;

		obsoleteFolderCheckTime.update();
		long earliestMinute = obsoleteFolderCheckTime.epochMicroseconds()/60000000 - _maxFramePeriods * 60;

		Poco::Path folderPath = _frameRootFolder;
		folderPath.pushDirectory(name());
		Poco::DirectoryIterator it(folderPath);
		Poco::DirectoryIterator end;

		for(; it!=end; it++)
		{
			if(it->isDirectory() == false) {
				continue;
			}

			int minute = std::atoi(it.name().c_str());
			if(minute > earliestMinute) {
				continue;
			}
			folderNames.push_back(it.name());
		}

		for(int i=0; i<folderNames.size(); i++)
		{
			Poco::Path folderToDelete = folderPath;
			folderToDelete.pushDirectory(folderNames[i]);
			try
			{
				Poco::File folder(folderToDelete);
				if(folder.exists()) {
					pLogger->LogInfo("delete folder: " + folderToDelete.toString());
					folder.remove(true);
				}
			}
			catch(...) {
				//do nothing
			}
		}
	}
};

class PersistanceAllocator: public Poco::Task
{
public:
	PersistanceAllocator(): Task("PersistanceAllocator") { }

private:

	virtual void runTask() override
	{
		pLogger->LogInfo("PersistanceAllocator " + name() + "starts");

		for(;;)
		{
			if(isCancelled()) {
				break;
			}
			if(_pCache->pFrameAvailableSemaphore->tryWait(1000) == false) {
				continue;
			}

			{
				Poco::ScopedLock<Poco::Mutex> lock(_pCache->mutex);
				for(; _pCache->availableFrameIndexes.empty() == false;)
				{
					auto frameIndex = _pCache->availableFrameIndexes[0];
					auto pFrame = _pCache->pendingFilePtrArray[frameIndex];

					_pCache->availableFrameIndexes.pop_front();

					if(pFrame->state == PendingFile::READY)
					{
						int taskIndex;
						pFrame->state = PendingFile::PERSISTING;

						//assign it to corresponding persistence task
						for(taskIndex=0; taskIndex<_persistanceTaskList.size(); taskIndex++)
						{
							auto pTask = _persistanceTaskList[taskIndex];
							if(pFrame->clientId == pTask->name()) {
								pTask->AddPendingFileIndex(frameIndex);
								break;
							}
						}
						if(taskIndex == _persistanceTaskList.size()) {
							//no available task is found
							pFrame->state = PendingFile::IDLE; //ignore the frame data.
						}
					}
					else
					{
						pLogger->LogError("PersistanceAllocator frame state not ready: " + std::to_string(pFrame->state));
					}
				}
			}
		}

		pLogger->LogInfo("PersistanceAllocator " + name() + "exits");
	}
};

class FrameFileHandler: public Poco::Net::PartHandler
{
public:
	FrameFileHandler():
		_length(0)
	{
	}

	void handlePart(const MessageHeader& header, std::istream& stream)
	{
		_type = header.get("Content-Type", "(unspecified)");
		if (header.has("Content-Disposition"))
		{
			std::string disp;
			NameValueCollection params;
			MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
			_name = params.get("name", "(unnamed)");
			_fileName = params.get("filename", "(unnamed)");
		}

		CountingInputStream istr(stream);
		NullOutputStream ostr;
		StreamCopier::copyStream(istr, ostr);
		_length = istr.chars();
	}

	int length() const
	{
		return _length;
	}

	const std::string& name() const
	{
		return _name;
	}

	const std::string& fileName() const
	{
		return _fileName;
	}

	const std::string& contentType() const
	{
		return _type;
	}

private:
	int _length;
	std::string _type;
	std::string _name;
	std::string _fileName;
};


class UploadRequestHandler: public HTTPRequestHandler
	/// Return a HTML document with the current date and time.
{
public:
	UploadRequestHandler()
	{
	}

	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		Application& app = Application::instance();
		app.logger().information("Request from " + request.clientAddress().toString());

		FrameFileHandler partHandler;
		HTMLForm form(request, request.stream(), partHandler);

		response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");

		std::ostream& ostr = response.send();

		ostr <<
			"<html>\n"
			"<head>\n"
			"<title>POCO Form Server Sample</title>\n"
			"</head>\n"
			"<body>\n"
			"<h1>POCO Form Server Sample</h1>\n"
			"<h2>GET Form</h2>\n"
			"<form method=\"GET\" action=\"/form\">\n"
			"<input type=\"text\" name=\"text\" size=\"31\">\n"
			"<input type=\"submit\" value=\"GET\">\n"
			"</form>\n"
			"<h2>POST Form</h2>\n"
			"<form method=\"POST\" action=\"/form\">\n"
			"<input type=\"text\" name=\"text\" size=\"31\">\n"
			"<input type=\"submit\" value=\"POST\">\n"
			"</form>\n"
			"<h2>File Upload</h2>\n"
			"<form method=\"POST\" action=\"/form\" enctype=\"multipart/form-data\">\n"
			"<input type=\"file\" name=\"file\" size=\"31\"> \n"
			"<input type=\"submit\" value=\"Upload\">\n"
			"</form>\n";

		ostr << "<h2>Request</h2><p>\n";
		ostr << "Method: " << request.getMethod() << "<br>\n";
		ostr << "URI: " << request.getURI() << "<br>\n";
		NameValueCollection::ConstIterator it = request.begin();
		NameValueCollection::ConstIterator end = request.end();
		for (; it != end; ++it)
		{
			ostr << it->first << ": " << it->second << "<br>\n";
		}
		ostr << "</p>";

		if (!form.empty())
		{
			ostr << "<h2>Form</h2><p>\n";
			it = form.begin();
			end = form.end();
			for (; it != end; ++it)
			{
				ostr << it->first << ": " << it->second << "<br>\n";
			}
			ostr << "</p>";
		}

		if (!partHandler.name().empty())
		{
			ostr << "<h2>Upload</h2><p>\n";
			ostr << "Name: " << partHandler.name() << "<br>\n";
			ostr << "File Name: " << partHandler.fileName() << "<br>\n";
			ostr << "Type: " << partHandler.contentType() << "<br>\n";
			ostr << "Size: " << partHandler.length() << "<br>\n";
			ostr << "</p>";
		}
		ostr << "</body>\n";
	}
};


class UploadRequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	UploadRequestHandlerFactory()
	{
	}

	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		return new UploadRequestHandler;
	}
};


class FrameServer: public Poco::Util::ServerApplication
	/// The main application class.
	///
	/// This class handles command-line arguments and
	/// configuration files.
	/// Start the HTTPFormServer executable with the help
	/// option (/help on Windows, --help on Unix) for
	/// the available command line options.
	///
	/// To use the sample configuration file (HTTPFormServer.properties),
	/// copy the file to the directory where the HTTPFormServer executable
	/// resides. If you start the debug version of the HTTPFormServer
	/// (HTTPFormServerd[.exe]), you must also create a copy of the configuration
	/// file named HTTPFormServerd.properties. In the configuration file, you
	/// can specify the port on which the server is listening (default
	/// 9980) and the format of the date/Form string sent back to the client.
	///
	/// To test the FormServer you can use any web browser (http://localhost:9980/).
{
public:
	FrameServer(): _helpRequested(false)
	{
	}

	~FrameServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A web server that shows how to work with HTML forms.");
		helpFormatter.format(std::cout);
	}

	void freeCache()
	{
		if(_pCache != nullptr)
		{
			if(_pCache->pFrameAvailableSemaphore != nullptr) {
				delete _pCache->pFrameAvailableSemaphore;
			}

			for(int i=0; i<_pCache->pendingFilePtrArray.size(); i++)
			{
				auto p = _pCache->pendingFilePtrArray[i];
				if(p->pData != NULL) {
					free(p->pData);
				}
				delete p;
			}

			delete _pCache;
			_pCache = nullptr;
		}
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			unsigned short port;
			int requestServiceThreadAmount;
			int maxQueuedRequest;
			unsigned int maxPendingFileAmount;
			unsigned int maxFileSize;
			bool bException = false;
			bool bMemoryShortage = false;
			HTTPServerParams * pServerParams;

			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;

			Poco::TaskManager tmLogger;

			//retrieve configuration
			try
			{
				port = (unsigned short) config().getInt("server_port.port", 9980);
				requestServiceThreadAmount = config().getInt("max_request_service_thread_amount");
				maxQueuedRequest = config().getInt("max_queued_request");
				maxPendingFileAmount = config().getInt("max_pending_file_amount");
				maxFileSize = config().getInt("max_file_size");

				_frameRootFolder = config().getString("frames_root_folder");
				_maxFramePeriods = config().getInt("max_frame_period_hours");

				for(int i=0; ;i++)
				{
					std::string key = "client_id_" + std::to_string(i);
					if(config().has(key))
					{
						std::string id = config().getString(key);
						_clientIds.push_back(id);
					}
					else {
						break;
					}
				}
				if(_clientIds.empty()) {
					std::cout << "No client is specified in configuration" << "\r\n";
					return Application::EXIT_CONFIG;
				}

				logFolder = config().getString("log_file_folder", "./logs/ImageCapture");
				logFile = config().getString("log_file_name", "ImageCapture");
				logFileSize = config().getString("log_file_size", "1M");
				logFileAmount = config().getString("log_file_amount", "10");
			}
			catch(Poco::Exception & e)
			{
				std::cout << "Exception: " << e.displayText() << "\r\n";
				bException = true;
			}
			if(bException) {
				return Application::EXIT_CONFIG;
			}

			//start the logger
			pLogger = new Logger(logFolder, logFile, logFileSize, logFileAmount);
			pLogger->CopyToConsole(true);
			tmLogger.start(pLogger);
			pLogger->LogInfo("**** FrameServer version 1.0.0 ****");

			//init _pCache
			bMemoryShortage = false;
			try
			{
				_pCache = new PendingFileCache;
				if(_pCache == nullptr) {
					throw Poco::Exception("memory shortage");
				}

				_pCache->pFrameAvailableSemaphore = new Poco::Semaphore(0, maxPendingFileAmount);
				if(_pCache->pFrameAvailableSemaphore == nullptr) {
					throw Poco::Exception("memory shortage");
				}

				for(int i=0; i<maxPendingFileAmount; i++)
				{
					PendingFile * p = new PendingFile;
					if(p == nullptr) {
						throw Poco::Exception("memory shortage");
					}
					p->maxSize = maxFileSize;
					p->pData = (unsigned char *)malloc(maxFileSize);
					if(p->pData == NULL) {
						throw Poco::Exception("memory shortage");
					}
					_pCache->pendingFilePtrArray.push_back(p);
				}
			}
			catch(Poco::Exception & e)
			{
				bMemoryShortage = true;
				pLogger->LogError("FrameServer memory shortage in cache initialization");
			}
			if(bMemoryShortage) {
				freeCache();
			}
			else
			{
				pServerParams = new HTTPServerParams;
				pServerParams->setMaxThreads(requestServiceThreadAmount);
				pServerParams->setMaxQueued(maxQueuedRequest);

				// set-up a server socket
				ServerSocket svs(port);
				// set-up a HTTPServer instance
				HTTPServer srv(new UploadRequestHandlerFactory, svs, pServerParams);
				// start the HTTPServer
				srv.start();
				// wait for CTRL-C or kill
				waitForTerminationRequest();
				// Stop the HTTPServer
				srv.stop();
			}

			//stop logger
			tmLogger.cancelAll();
			tmLogger.joinAll();

			freeCache();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


int main(int argc, char** argv)
{
	FrameServer app;
	return app.run(argc, argv);
}
