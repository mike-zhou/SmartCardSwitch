/***************************************************************************
 *   v4l2grab Version 0.3                                                  *
 *   Copyright (C) 2012 by Tobias Müller                                   *
 *   Tobias_Mueller@twam.info                                              *
 *                                                                         *
 *   based on V4L2 Specification, Appendix B: Video Capture Example        *
 *   (http://v4l2spec.bytesex.org/spec/capture-example.html)               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 /**************************************************************************
 *   Modification History                                                  *
 *                                                                         *
 *   Matthew Witherwax      21AUG2013                                      *
 *      Added ability to change frame interval (ie. frame rate/fps)        *
 * Martin Savc              7JUL2015
 *      Added support for continuous capture using SIGINT to stop.
 ***************************************************************************/

// compile with all three access methods
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <jpeglib.h>
#include <libv4l2.h>
#include <signal.h>
#include <stdint.h>
#include <inttypes.h>
#include <iostream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Exception.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/Mutex.h"
#include "Poco/Semaphore.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/FilePartSource.h"
#include "Poco/ThreadPool.h"

#include "Logger.h"

#include "config.h"
#include "yuv.h"

using Poco::Util::Application;
using Poco::Util::ServerApplication;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Task;
using Poco::TaskManager;
using Poco::DateTimeFormatter;

Logger * pLogger;

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
        IO_METHOD_READ,
} io_method;

enum FrameDataState
{
	IDLE = 0,
	CAPTURING,
	RAW_DATA_READY,
	DECODING
};

struct FrameData
{
	enum FrameDataState state;

	int dataSize;
	void * pRawFrame;
	void * pDecodedFrame;

	Poco::Timestamp stamp;
};

struct FrameCache
{
	FrameCache()
	{
		pSemaphore = NULL;
		frameAmount = 0;
		pFrames = NULL;
	}

	~FrameCache()
	{
		delete pSemaphore;
	}

	Poco::Mutex mutex;
	Poco::Semaphore * pSemaphore;
	int frameAmount;
	struct FrameData * pFrames;
} _frameCache;

static int _fd = -1;

// global settings
static unsigned int _width;
static unsigned int _height;
static int _fps;
static unsigned char _jpegQuality;
static std::string _deviceFile;
std::string _ramdiskFolder;
std::string _hostServerIp;
int _hostServerPort;
std::string _hostServerApi;
std::string _monitorId;
int _uploadTimeout;

/**
	Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

	\param _fd file descriptor
	\param request request
	\param argp argument
	\returns result from ioctl
*/
static int xioctl(int fd, int request, void* argp)
{
	int r;

	do r = v4l2_ioctl(fd, request, argp);
	while (-1 == r && EINTR == errno);

	return r;
}

/**
	Write image to jpeg file.

	\param img image to write
*/
static void jpegWrite(unsigned char* img, const char* jpegFilename)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];
	FILE *outfile = fopen( jpegFilename, "wb" );

	// try to open file for saving
	if (!outfile) {
		pLogger->LogError("jpegWrite unable to open jpeg file");
		return;
	}

	// create jpeg data
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	// set image parameters
	cinfo.image_width = _width;
	cinfo.image_height = _height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;

	// set jpeg compression parameters to default
	jpeg_set_defaults(&cinfo);
	// and then adjust quality setting
	jpeg_set_quality(&cinfo, _jpegQuality, TRUE);

	// start compress
	jpeg_start_compress(&cinfo, TRUE);

	// feed data
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &img[cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	// finish compression
	jpeg_finish_compress(&cinfo);

	// destroy jpeg data
	jpeg_destroy_compress(&cinfo);

	// close output file
	fclose(outfile);
}

/**
	read single frame
*/
static int frameRead(void)
{
	struct FrameData * pFrameData = NULL;

	{
		Poco::ScopedLock<Poco::Mutex> lock(_frameCache.mutex);
		for(int i=0; i<_frameCache.frameAmount; i++)
		{
			if(_frameCache.pFrames[i].state == IDLE)
			{
				pFrameData = &(_frameCache.pFrames[i]);
				pFrameData->state = CAPTURING;
				break;
			}
		}
	}

	if(pFrameData == NULL) {
		pLogger->LogError("frameRead no idle frame cache");
	}
	else
	{
		if (-1 == v4l2_read(_fd, pFrameData->pRawFrame, pFrameData->dataSize))
		{
			{
				Poco::ScopedLock<Poco::Mutex> lock(_frameCache.mutex);

				pFrameData->state = IDLE;
				memset(pFrameData->pRawFrame, 0, pFrameData->dataSize);
			}

			switch (errno) {
				case EAGAIN:
					return 0;

				case EIO:
					// Could ignore EIO, see spec.
					// fall through

				default:
					throw Poco::Exception("failed to read from device");
			}
		}
		else
		{
			Poco::ScopedLock<Poco::Mutex> lock(_frameCache.mutex);

			pFrameData->stamp.update();
			pFrameData->state = RAW_DATA_READY;
			_frameCache.pSemaphore->set();//trigger an uploading task.
		}
	}

	return 1;
}

/**
	mainloop: read frames and process them
	return value:
		true: 	a frame is processed successfully
		false:	error occurs
*/
static bool captureImage(void)
{	
	for (;;)
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(_fd, &fds);

		/* Timeout. */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		r = select(_fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;

			pLogger->LogError("error in waiting device file");
			return false;
		}

		if (0 == r)
		{
			pLogger->LogError("device time out");
			return false;
		}

		frameRead();

		break;
	}

	return true;
}

/**
	initialize device
*/
static void deviceInit(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_streamparm frameint;
	unsigned int min;

	if (-1 == xioctl(_fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			throw Poco::Exception("none v4l2 device");
		} else {
			throw Poco::Exception("error in QUERYCAP: " + std::to_string(errno));
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		throw Poco::Exception("none video capture device");
	}

	if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
		throw Poco::Exception("device doesn't support READWRITE");
	}

	/* Select video input, video standard and tune here. */
	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(_fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(_fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
				case EINVAL:
					/* Cropping not supported. */
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	} else {
		/* Errors ignored. */
	}

	CLEAR(fmt);

	// v4l2_format
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = _width;
	fmt.fmt.pix.height = _height;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;

	if (-1 == xioctl(_fd, VIDIOC_S_FMT, &fmt)){
		throw Poco::Exception("VIDIOC_S_FMT");
	}

	if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_YUV420) {
		throw Poco::Exception("Libv4l didn't accept YUV420 format. Can't proceed");
	}

	/* Note VIDIOC_S_FMT may change width and height. */
	if (_width != fmt.fmt.pix.width)
	{
		_width = fmt.fmt.pix.width;
		pLogger->LogInfo("Image width set to " + std::to_string(_width) + " by device: " + _deviceFile);
	}

	if (_height != fmt.fmt.pix.height)
	{
		_height = fmt.fmt.pix.height;
		pLogger->LogInfo("Image height set to " + std::to_string(_height) + " by device: " + _deviceFile);
	}
	
	/* If the user has set the fps to -1, don't try to set the frame interval */
	if (_fps != -1)
	{
		CLEAR(frameint);

		/* Attempt to set the frame interval. */
		frameint.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		frameint.parm.capture.timeperframe.numerator = 1;
		frameint.parm.capture.timeperframe.denominator = _fps;
		if (-1 == xioctl(_fd, VIDIOC_S_PARM, &frameint))
		{
		   pLogger->LogError("failed to set fps to: " + std::to_string(_fps));
		}
	}
}

/**
	close device
*/
static void deviceClose(void)
{
	if (-1 == v4l2_close(_fd))
	{
		pLogger->LogError("failed to close device file");
	}

	_fd = -1;
}

/**
	open device
*/
static void deviceOpen(void)
{
	struct stat st;

	// stat file
	if (-1 == stat(_deviceFile.c_str(), &st)) {
		throw Poco::Exception("failed to retrieve device stat: " + _deviceFile);
	}

	// check if its device
	if (!S_ISCHR(st.st_mode)) {
		throw Poco::Exception(" none video device: " + _deviceFile);
	}

	// open device
	_fd = v4l2_open(_deviceFile.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

	// check if opening was successful
	if (-1 == _fd) {
		throw Poco::Exception("failed to open device: " + _deviceFile);;
	}
}

class CaptureTask: public Poco::Task
{
public:
	CaptureTask():Task("ImageCapture") {}
	
private:
	Poco::Timestamp _lastFolderDeletionTime;

	void runTask()
	{
		try
		{
			bool errorOccured = false;

			// open and initialize device
			deviceOpen();
			deviceInit();

			while(1)
			{
				if(isCancelled())
				{
					break;
				}
				else
				{
					if(errorOccured)
					{
						deviceOpen();
						deviceInit();
					}
					// capture a frame
					if(captureImage() == false)
					{
						deviceClose();
						errorOccured = true;
						sleep(5000);
					}
				}

			}

			// close device
			deviceClose();
		}
		catch(Poco::Exception &e)
		{
			pLogger->LogError("CaptureTask exception: " + e.displayText());
		}
		catch(std::exception & e)
		{
			pLogger->LogError("CaptureTask exception: " + std::string(e.what()));
		}
		catch(...)
		{
			pLogger->LogError("CaptureTask unknown exception");
		}
		pLogger->LogInfo("CaptureTask terminates");
	}
};

class UploadingTask: public Poco::Task
{
public:
	UploadingTask():Task("uploading")
	{
	}

private:
	virtual void runTask() override
	{
		for(;;)
		{
			struct FrameData * pFrame = NULL;
			long milliseconds;
			char fileName[64];
			Poco::Path jpegFilePath = _ramdiskFolder;

			_frameCache.pSemaphore->wait();

			if(isCancelled()) {
				break;
			}

			//find a pending frame
			{
				Poco::ScopedLock<Poco::Mutex> lock(_frameCache.mutex);
				for(int i=0; i<_frameCache.frameAmount; i++) {
					if(_frameCache.pFrames[i].state == RAW_DATA_READY) {
						pFrame = &(_frameCache.pFrames[i]);
						_frameCache.pFrames[i].state = DECODING;
						break;
					}
				}
			}

			if(pFrame == NULL) {
				pLogger->LogError("UploadingTask: no raw frame is ready");
				continue;
			}

			//transcoding
			YUV420toYUV444(_width, _height, (unsigned char *)(pFrame->pRawFrame), (unsigned char *)(pFrame->pDecodedFrame));

			//write to JPEG file
			milliseconds = pFrame->stamp.raw()/1000;
			sprintf(fileName, "%010ld.jpg", milliseconds);
			jpegFilePath.setFileName(fileName);
			jpegWrite((unsigned char *)(pFrame->pDecodedFrame), jpegFilePath.toString().c_str());

			//upload JPEG file
			Poco::File jpegFile(jpegFilePath);
			if(jpegFile.exists()) {
				uploadFile(jpegFilePath.toString());
				jpegFile.remove(false); //delete the file
			}
			else {
				pLogger->LogError("UploadingTask: failed to save jpeg file: " + jpegFilePath.toString());
			}

			//reset frame data
			memset(pFrame->pRawFrame, 0, pFrame->dataSize);
			memset(pFrame->pDecodedFrame, 0, pFrame->dataSize);
			pFrame->state = IDLE;
		}

		pLogger->LogInfo("uploading task exit");
	}

	void uploadFile(const std::string & fileName)
	{
		try
		{
			Poco::Path path(fileName);
			Poco::Net::HTTPClientSession session(_hostServerIp, _hostServerPort);
			Poco::Net::HTTPRequest request;
			Poco::Net::HTMLForm form;
			Poco::Net::HTTPResponse response;

			request.setURI(_hostServerApi);
			request.setMethod(Poco::Net::HTTPRequest::HTTP_POST);
			request.setContentType("application/octet-stream");

			form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
			form.addPart("file", new Poco::Net::FilePartSource(fileName));
			form.add("monitorId", _monitorId);
			form.add("frameName", path.getFileName());
			form.prepareSubmit(request);

			session.setTimeout(Poco::Timespan(_uploadTimeout, 0));
			std::ostream & outputStream = session.sendRequest(request);
			form.write(outputStream);

			//http response is ignored.
			session.receiveResponse(response);
		}
		catch(Poco::Exception & e)
		{
			pLogger->LogError("uploadFile exception: " + e.displayText());
		}
		catch(std::exception & e)
		{
			pLogger->LogError("uploadFile exception: " + std::string(e.what()));
		}
		catch(...)
		{
			pLogger->LogError("uploadFile unknown exception");
		}
	}
};


class ImageCapture: public ServerApplication
{
public:
	ImageCapture(): _helpRequested(false)
	{
	}

	~ImageCapture()
	{
	}

protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
		logger().information("starting up");
	}

	void uninitialize()
	{
		logger().information("shutting down");
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false)
				.callback(OptionCallback<ImageCapture>(this, &ImageCapture::handleHelp)));
	}

	void handleHelp(const std::string& name, const std::string& value)
	{
		_helpRequested = true;
		displayHelp();
		stopOptionsProcessing();
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample server application that demonstrates some of the features of the Util::ServerApplication class.");
		helpFormatter.format(std::cout);
	}

	int main(const ArgVec& args)
	{
		if(_helpRequested) {
			return Application::EXIT_OK;
		}

		Poco::ThreadPool uploadThreadPool(1, 64);
		TaskManager tmLogger;
		TaskManager tm(uploadThreadPool);
		std::string logFolder;
		std::string logFile;
		std::string logFileSize;
		std::string logFileAmount;
		int cacheSize;
		bool bMemoryShortage = false;

		//use the designated configuration if it exist
		if(args.size() > 0)
		{
			std::string configFile = args[0];
			Poco::Path path(configFile);

			if(path.isFile())
			{
				Poco::File file(path);

				if(file.exists() && file.canRead()) {
					loadConfiguration(configFile);
				}
			}
		}

		try
		{
			//logs
			logFolder = config().getString("log_file_folder", "./logs/ImageCapture");
			logFile = config().getString("log_file_name", "ImageCapture");
			logFileSize = config().getString("log_file_size", "1M");
			logFileAmount = config().getString("log_file_amount", "10");

			_deviceFile = config().getString("device_file", "/dev/video0");
			_width = config().getUInt("width", 640);
			_height = config().getUInt("height", 480);
			_fps = config().getInt("fps", 30);
			_jpegQuality = config().getUInt("quality", 70);

			cacheSize = config().getInt("frame_cache_size", 30);
			_ramdiskFolder = config().getString("ramdisk_folder");

			_hostServerIp = config().getString("host_server_ip");
			_hostServerPort = config().getInt("host_server_port");
			_hostServerApi = config().getString("host_server_api");
			_uploadTimeout = config().getInt("upload_time_out");

			_monitorId = config().getString("monitor_id");
		}
		catch(Poco::NotFoundException& e)
		{
			logger().error("Config NotFoundException: " + e.displayText());
		}
		catch(Poco::SyntaxException& e)
		{
			logger().error("Config SyntaxException: " + e.displayText());
		}
		catch(Poco::Exception& e)
		{
			logger().error("Config Exception: " + e.displayText());
		}
		catch(...)
		{
			logger().error("Config unknown exception");
		}

		//start the logger
		pLogger = new Logger(logFolder, logFile, logFileSize, logFileAmount);
		pLogger->CopyToConsole(true);
		tmLogger.start(pLogger);
		pLogger->LogInfo("**** ImageCapture version 1.0.0 ****");

		try
		{
			_frameCache.pSemaphore = new Poco::Semaphore(0, cacheSize);
		}
		catch(Poco::Exception & e)
		{
			pLogger->LogError("failed to create semaphore: " + e.displayText());
		}
		catch(std::exception & e)
		{
			pLogger->LogError("failed to create semaphore: " + std::string(e.what()));
		}
		catch(...)
		{
			pLogger->LogError("failed to create semaphore");
		}
		//allocate memory
		_frameCache.frameAmount = cacheSize;
		_frameCache.pFrames = (FrameData *)malloc(sizeof(FrameData) * cacheSize);
		if(_frameCache.pFrames == NULL)
		{
			bMemoryShortage = true;
			pLogger->LogError("Not enough memory");
		}
		else
		{
			for(int i=0; i<cacheSize; i++)
			{
				_frameCache.pFrames[i].pRawFrame = NULL;
				_frameCache.pFrames[i].pDecodedFrame = NULL;
			}

			for(int i=0; i<cacheSize; i++)
			{
				_frameCache.pFrames[i].dataSize = _width * _height * 4;

				_frameCache.pFrames[i].pRawFrame = malloc(_frameCache.pFrames[i].dataSize);
				if(_frameCache.pFrames[i].pRawFrame == NULL) {
					bMemoryShortage = true;
					pLogger->LogError("Not enough memory");
					break;
				}

				_frameCache.pFrames[i].pDecodedFrame = malloc(_frameCache.pFrames[i].dataSize);
				if(_frameCache.pFrames[i].pDecodedFrame == NULL) {
					bMemoryShortage = true;
					pLogger->LogError("Not enough memory");
					break;
				}

				_frameCache.pFrames[i].state = IDLE;
			}
		}

		//clean up frames in _ramdiskFolder
		{
			std::vector<std::string> frameFiles;
			Poco::DirectoryIterator it(_ramdiskFolder);
			Poco::DirectoryIterator end;

			for(; it != end; it++) {
				frameFiles.push_back(it.name());
			}
			for(int i=0; i<frameFiles.size(); i++)
			{
				pLogger->LogInfo("delete obsolete frame file: " + frameFiles[i]);

				if(frameFiles[i].empty()) {
					continue;
				}
				Poco::Path path(_ramdiskFolder);

				path.setFileName(frameFiles[i]);
				Poco::File file(path);
				if(file.exists()) {
					file.remove(false);
				}
			}
		}

		if(!bMemoryShortage)
		{
			//start upload tasks
			for(int i=0; i<cacheSize; i++)
			{
				bool exceptionOccured = false;
				UploadingTask * pTask = NULL;
				try
				{
					pTask = new UploadingTask;

					if(pTask == NULL) {
						pLogger->LogError("Failed to allocate uploading task");
						exceptionOccured = true;
					}
					else {
						tm.start(pTask);
					}
				}
				catch(Poco::Exception &e)
				{
					pLogger->LogError("Exception in creating uploading task: " + e.displayText());
					exceptionOccured = true;
				}
				catch(...)
				{
					pLogger->LogError("Unknown exception in creating uploading task");
					exceptionOccured = true;
				}

				if(exceptionOccured) {
					pLogger->LogInfo("Uploading task amount: " + std::to_string(i));
					break;
				}
			}

			//start the CaptureTask
			CaptureTask * pTask = new CaptureTask();
			tm.start(pTask);

			waitForTerminationRequest();

			//stop tasks
			pLogger->LogInfo("**** ImageCapture Exiting ****");
			tm.cancelAll();
			for(int i=0; i<_frameCache.frameAmount; i++) {
				_frameCache.pSemaphore->set(); //to terminate all uploading tasks.
			}
			tm.joinAll();

			//stop logger
			tmLogger.cancelAll();
			tmLogger.joinAll();

			//free memory
			if(_frameCache.pFrames != NULL)
			{
				for(int i=0; i<cacheSize; i++)
				{
					if(_frameCache.pFrames[i].pRawFrame != NULL) {
						free(_frameCache.pFrames[i].pRawFrame);
					}
					if(_frameCache.pFrames[i].pDecodedFrame != NULL) {
						free(_frameCache.pFrames[i].pDecodedFrame);
					}
				}
				free(_frameCache.pFrames);
			}
		}

		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(ImageCapture)
