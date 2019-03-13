/*
 * CDeviceMonitor.h
 *
 *  Created on: Mar 13, 2019
 *      Author: mikez
 */

#ifndef CDEVICEMONITOR_H_
#define CDEVICEMONITOR_H_

#include <vector>
#include <deque>
#include <string>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"

class CDeviceMonitor: public Poco::Task
{
public:
	CDeviceMonitor(const std::string& filePath);

private:
	virtual void runTask() override;

	void onDeviceCanBeRead(int fd);

	std::string _deviceFile;
	static const int BUFFER_LENGTH = 1024;
	unsigned char _buffer[BUFFER_LENGTH];
};

#endif /* CDEVICEMONITOR_H_ */
