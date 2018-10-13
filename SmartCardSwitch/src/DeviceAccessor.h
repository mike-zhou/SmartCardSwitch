/*
 * DeviceAccessor.h
 *
 *  Created on: Oct 13, 2018
 *      Author: user1
 */

#ifndef DEVICEACCESSOR_H_
#define DEVICEACCESSOR_H_

#include <memory>
#include <deque>
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Task.h"
#include "Poco/Mutex.h"


class IDeviceObserver
{
public:
	virtual void OnFeedback(const std::string& feedback) = 0;

	virtual ~IDeviceObserver() {}
};

class DeviceAccessor: public Poco::Task
{
public:
	DeviceAccessor();
	~DeviceAccessor() {}

	// return value:
	// 		true: connection to deviceAddress succeeded
	//		false:	connection to deviceAddress failed
	bool Init(const Poco::Net::SocketAddress& deviceAddress);

	bool ReConnect();

	// send a string message to device
	// return value:
	// 		true: cmd can be sent out
	//		false: cmd cannot be sent out
	bool SendCommand(const std::string& cmd);

	void AddObserver(std::shared_ptr<IDeviceObserver> observerPtr);

	void runTask();

private:
	static const char * MSG_DEVICE_DISCONNECTED;

	Poco::Mutex _mutex;

	bool _connected;

	Poco::Net::SocketAddress _socketAddress;
	Poco::Net::StreamSocket _socket;
	std::deque<unsigned char> _incoming; //incoming data from socket
	std::deque<unsigned char> _outgoing; //outgoing data to socket, one command at most at any time

	std::vector<std::shared_ptr<IDeviceObserver>> _observerPtrArray;

	void onIncoming();
};



#endif /* DEVICEACCESSOR_H_ */
