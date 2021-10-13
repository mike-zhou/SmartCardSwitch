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
#include "Poco/TaskManager.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "ILowlevelDevice.h"

class CDeviceMonitor: public Poco::Task, public ILowlevelDeviceObsesrver
{
public:
	CDeviceMonitor(const std::string& filePath);

private:
	virtual void runTask() override;

	virtual void onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info) override;
	virtual void onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice) override;
	virtual void onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data) override;

	void onMonitorCanBeRead(std::deque<unsigned char> & reply);

	std::string _deviceFile;
	static const int BUFFER_LENGTH = 1024;
	unsigned char _buffer[BUFFER_LENGTH];

	Poco::TaskManager _tm;
};

#endif /* CDEVICEMONITOR_H_ */
