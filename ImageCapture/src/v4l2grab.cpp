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

struct buffer {
        void *                  start;
        size_t                  length;
};

static int              fd              = -1;
struct buffer *         buffers         = NULL;

// global settings
static unsigned int _width;
static unsigned int _height;
static int _fps;
static unsigned char _jpegQuality;
static std::string _deviceFile;
static unsigned char * _pYUV422Buffer;
static Poco::Path _framesRootFolder;
static unsigned int _record_period_hours = 1;

/**
	Do ioctl and retry if error was EINTR ("A signal was caught during the ioctl() operation."). Parameters are the same as on ioctl.

	\param fd file descriptor
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
static void jpegWrite(unsigned char* img, char* jpegFilename)
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
	process image read
*/
static void imageProcess(const void* p)
{
	char fileName[256];
	char folderName[32];
	unsigned char* src = (unsigned char*)p;
	Poco::Timestamp curTime;
	long milliseconds = curTime.raw()/1000;
	int minutes = milliseconds/60000;
	Poco::Path frameFolder = _framesRootFolder;

	YUV420toYUV444(_width, _height, src, _pYUV422Buffer);

	sprintf(folderName, "%010d", minutes);
	sprintf(fileName, "%010ld.jpg", milliseconds);

	frameFolder.pushDirectory(folderName);

	try
	{
		Poco::File folder (frameFolder);
		if(!folder.exists()) {
			pLogger->LogInfo("create folder: " + folder.path());
			folder.createDirectories();
		}

		frameFolder.setFileName(fileName);
		sprintf(fileName, "%s", frameFolder.toString().c_str());
		// write jpeg
		jpegWrite(_pYUV422Buffer, fileName);
	}
	catch(...) {
		//do nothing.
	}
}

/**
	read single frame
*/
static int frameRead(void)
{
	if (-1 == v4l2_read(fd, buffers[0].start, buffers[0].length)) {
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

	imageProcess(buffers[0].start);

	return 1;
}

/**
	mainloop: read frames and process them
*/
static void captureImage(void)
{	
	for (;;)
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* Timeout. */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;

			throw Poco::Exception("error in waiting device file");
		}

		if (0 == r)
		{
			throw Poco::Exception("device time out");
		}

		frameRead();

		break;
	}
}

static void deviceUninit(void)
{
	free(buffers[0].start);
	free(buffers);
}

static void readInit(unsigned int buffer_size)
{
	buffers = (struct buffer *)calloc(1, sizeof(*buffers));

	if (!buffers)
	{
		throw Poco::Exception("Out of memory");
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		throw Poco::Exception("Out of memory");
	}
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

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
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

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
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

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt)){
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
		if (-1 == xioctl(fd, VIDIOC_S_PARM, &frameint))
		{
		   pLogger->LogError("failed to set fps to: " + std::to_string(_fps));
		}
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	readInit(fmt.fmt.pix.sizeimage);
}

/**
	close device
*/
static void deviceClose(void)
{
	if (-1 == v4l2_close(fd))
	{
		pLogger->LogError("failed to close device file");
	}

	fd = -1;
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
	fd = v4l2_open(_deviceFile.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

	// check if opening was successful
	if (-1 == fd) {
		throw Poco::Exception("failed to open device: " + _deviceFile);;
	}
}

class CaptureTask: public Poco::Task
{
public:
	CaptureTask():Task("ImageCapture") {}
	
private:
	Poco::Timestamp _lastFolderDeletionTime;

	void deleteObsoleteFrames()
	{
		Poco::Timestamp curTime;

		//delete obsolete folders every 1 minute
		if((curTime - _lastFolderDeletionTime) < 60000000) {
			return;
		}
		_lastFolderDeletionTime.update();

		std::vector<std::string> directoryNames;

		Poco::DirectoryIterator it(_framesRootFolder) ;
		Poco::DirectoryIterator end;
		long curMilliseconds = curTime.raw()/1000;
		int earliestMinutes = curMilliseconds/60000 - _record_period_hours*60;

		//find the directories to delete
		for(; it != end; it++) {
			if(it->isDirectory()) {
				int minutes = std::atoi(it.name().c_str());
				if(minutes > earliestMinutes) {
					continue;
				}
				directoryNames.push_back(it.name());
			}
		}
		//delete directories.
		for(int i=0; i<directoryNames.size(); i++)
		{
			Poco::Path frameFolder = _framesRootFolder;

			frameFolder.pushDirectory(directoryNames[i]);
			try
			{
				Poco::File folder (frameFolder);
				if(folder.exists()) {
					pLogger->LogInfo("delete folder: " + folder.path());
					folder.remove(true); //delete this folder
				}
			}
			catch(...) {
				//do nothing
			}
		}
	}

	void runTask()
	{
		_pYUV422Buffer = (unsigned char*)malloc(_width*_height*3*sizeof(char));
		if(NULL == _pYUV422Buffer) {
			pLogger->LogError("CaptureTask failed to allocate memory");
			return;
		}

		try
		{
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
					// capture a frame
					captureImage();
				}

				deleteObsoleteFrames();
			}

			// close device
			deviceUninit();
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

		if(_pYUV422Buffer != NULL) {
			free(_pYUV422Buffer);
		}

		pLogger->LogInfo("CaptureTask terminates");
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
		if (!_helpRequested)
		{
			TaskManager tmLogger;
			TaskManager tm;
			std::string logFolder;
			std::string logFile;
			std::string logFileSize;
			std::string logFileAmount;

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
				_framesRootFolder = config().getString("output_folder");
				_record_period_hours = config().getUInt("record_period_hours");
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

			//start the CaptureTask
			CaptureTask * pTask = new CaptureTask();
			tm.start(pTask);

			waitForTerminationRequest();

			//stop tasks
			pLogger->LogInfo("**** ImageCapture Exiting ****");
			tm.cancelAll();
			tm.joinAll();
			//stop logger
			tmLogger.cancelAll();
			tmLogger.joinAll();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};


POCO_SERVER_MAIN(ImageCapture)
