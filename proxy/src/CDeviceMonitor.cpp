/*
 * CDeviceMonitor.cpp
 *
 *  Created on: Mar 13, 2019
 *      Author: mikez
 */

#include "stddef.h"
#include "fcntl.h"
#include <poll.h>
#include <stddef.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "ProxyLogger.h"
#include "CDeviceMonitor.h"

extern ProxyLogger * pLogger;

CDeviceMonitor::CDeviceMonitor(const std::string& filePath): Task("CDeviceMonitor")
{
	_deviceFile = filePath;
}

void CDeviceMonitor::onDeviceCanBeRead(int fd)
{
	memset(_buffer, 0, BUFFER_LENGTH);
	auto amount = read(fd, _buffer, BUFFER_LENGTH);
	auto errorNumber = errno;

	if(amount == 0) {
		pLogger->LogError("CDeviceMonitor::onDeviceCanBeRead error: EOF is returned");
	}
	else if(amount < 0) {
		pLogger->LogError("CDeviceMonitor::onDeviceCanBeRead error number: " + std::to_string(errorNumber));
	}
	else
	{
		std::string outputStr;

		pLogger->LogInfo("CDeviceMonitor::onDeviceCanBeRead " + std::to_string(amount) + " bytes from: " + _deviceFile);
		for(int i=0; i<amount; i++)
		{
			char buf[32];

			sprintf(buf, "%02x,", _buffer[i]);
			outputStr = outputStr + std::string(buf);
		}
		pLogger->LogInfo("CDeviceMonitor::onDeviceCanBeRead hex content: " + outputStr);
		pLogger->LogInfo("CDeviceMonitor::onDeviceCanBeRead char content: " + std::string((char *)_buffer));
	}
}

void CDeviceMonitor::runTask()
{
	bool monitorOpened = false;
	int monitorFileDescriptor;

	pLogger->LogInfo("CDeviceMonitor::runTask start");

	if(_deviceFile.empty()) {
		pLogger->LogError("CDeviceMonitor::runTask no monitor device file set");
	}
	else
	{
		monitorFileDescriptor = open(_deviceFile.c_str(), O_RDONLY | O_NOCTTY);
		if(monitorFileDescriptor < 0){
			pLogger->LogError("CDeviceMonitor::runTask failed to open: " + _deviceFile);
		}
		else
		{
			struct termios tios;
			int rc;
			//change settings of device.
			rc = tcgetattr(monitorFileDescriptor, &tios);
			if(0 != rc)
			{
				auto e = errno;
				pLogger->LogError("CDeviceMonitor::runTask tcgetattr errno: " + std::to_string(e));
				close(monitorFileDescriptor);
			}
			else
			{
				//115200, 8, N, 1
				rc = cfsetspeed(&tios, B115200);
				if(0 != rc)
				{
					auto e = errno;
					pLogger->LogError("CDeviceMonitor::runTask cfsetspeed errno: " + std::to_string(e));
					close(monitorFileDescriptor);
				}
				else
				{
					//c_iflag
					tios.c_iflag &= ~ICRNL;
					tios.c_iflag &= ~IXON;
					tios.c_iflag |= IGNPAR;
					//c_oflag
					tios.c_oflag &= ~ONLCR;
					tios.c_oflag &= ~OPOST;
					//c_cflag
					tios.c_cflag &= ~CLOCAL;
					tios.c_cflag |= CS8;
					tios.c_cflag &= ~CSTOPB;
					tios.c_cflag &= ~PARENB;
					//c_lflag
					tios.c_lflag &= ~ISIG;
					tios.c_lflag &= ~ICANON;
					tios.c_lflag &= ~IEXTEN;
					tios.c_lflag &= ~ECHO;
					tios.c_lflag &= ~ECHOE;
					tios.c_lflag &= ~ECHOK;
					tios.c_lflag &= ~ECHOCTL;
					tios.c_lflag &= ~ECHOKE;
					tios.c_lflag &= ~FLUSHO;
					tios.c_lflag &= ~EXTPROC;

					rc = tcsetattr(monitorFileDescriptor, TCSANOW, &tios);
					if(0 != rc)
					{
						auto e = errno;
						pLogger->LogError("CDeviceMonitor::runTask tcsetattr errno: " + std::to_string(e));
						close(monitorFileDescriptor);
					}
					else
					{
						pLogger->LogInfo("CDeviceMonitor::runTask monitor file is opened");
						monitorOpened = true;
					}
				}
			}
		}
	}

	if(monitorOpened)
	{
		while(1)
		{
			if(isCancelled()) {
				break;
			}
			else
			{
				int rc;
				std::vector<struct pollfd> fdVector;

				{
					pollfd pfd;

					pfd.fd = monitorFileDescriptor;
					pfd.events = POLLIN | POLLERR;
					pfd.revents = 0;
					fdVector.push_back(pfd);
				}
				rc = poll(fdVector.data(), fdVector.size(), 10);
				auto errorNumber = errno;
				if(rc > 0)
				{
					auto events = fdVector[0].revents;

					if(events & POLLIN) {
						//device can be read.
						onDeviceCanBeRead(monitorFileDescriptor);
					}
					if(events & POLLERR)
					{
						pLogger->LogError("CDeviceMonitor::runTask error in device: " + _deviceFile);
						pLogger->LogError("CDeviceMonitor::runTask error number: " + std::to_string(errorNumber));
						break;
					}
				}
				else if(rc < 0)
				{
					pLogger->LogError("CDeviceMonitor::runTask error in polling device: " + _deviceFile);
					pLogger->LogError("CDeviceMonitor::runTask error number: " + std::to_string(errorNumber));
					break;
				}
			}
		}

		close(monitorFileDescriptor);
	}

	pLogger->LogInfo("CDeviceMonitor::runTask exited");
}
