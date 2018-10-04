/*
 * IDeviceObserver.h
 *
 *  Created on: Oct 5, 2018
 *      Author: user1
 */

#ifndef IDEVICEOBSERVER_H_
#define IDEVICEOBSERVER_H_

class IDeviceObserver
{
public:

	virtual void OnDeviceInserted(const std::string& deviceName) = 0;

	virtual void OnDeviceUnplugged(const std::string& deviceName) = 0;

	virtual void OnDeviceReply(const std::string& deviceName, const std::string& reply) = 0;

	virtual ~IDeviceObserver() {}
};



#endif /* IDEVICEOBSERVER_H_ */
