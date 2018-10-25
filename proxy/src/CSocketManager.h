/*
 * CSocketManager.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CSOCKETMANAGER_H_
#define CSOCKETMANAGER_H_

#include <map>
#include <vector>
#include <deque>
#include <memory>
#include "Poco/Task.h"
#include "Poco/Mutex.h"
#include "Poco/Net/StreamSocket.h"
#include "IDeviceObserver.h"
#include "IDevice.h"
#include "ISocketDeposit.h"
#include "CommandTranslater.h"


using Poco::Net::StreamSocket;

/******************
 * Functionalities of this class object
 * 1. maintain connections between sockets and devices
 * 2. translate JSON command to device command
 * 3. translate device reply to JSON reply
 ******************/
class CSocketManager : public Poco::Task, public IDeviceObserver, public ISocketDeposit {
public:
	CSocketManager();
	virtual ~CSocketManager();

	void SetDevice(IDevice * pDevice);

private:
	//IDeviceObserver
	virtual void OnDeviceInserted(const std::string& deviceName) override;
	virtual void OnDeviceUnplugged(const std::string& deviceName) override;
	virtual void OnDeviceReply(const std::string& deviceName, const std::string& reply) override;

	//ISocketDeposit
	virtual void AddSocket(StreamSocket& socket) override;

	void runTask();

private:
	Poco::Mutex _mutex;

	const long long INVALID_SOCKET_ID = -1;
	const long long STARTING_SOCKET_ID = 1;
	long long _lastSocketId;

	//a map of socket id and socket object
	enum SocketState
	{
		ACTIVE,
		TO_BE_CLOSED
	};
	struct SocketWrapper
	{
		long socketId;
		StreamSocket socket;
		enum SocketState state;
		std::deque<unsigned char> incoming;//reception stage to save partial command from socket
		std::deque<unsigned char> outgoing;//sending stage for formatted outgoing reply
	};
	std::vector<struct SocketWrapper> _sockets;


	//device has a 1:1 relationship to socket
	// a map of device name vs Device wrapper.
	struct DeviceWrapper
	{
		long long socketId;//which socket this device bonds to
		std::deque<std::string> replyPool; //to save information from device.
	};
	std::map<std::string, struct DeviceWrapper> _deviceSocketMap;
	IDevice * _pDevice;

	void onDeviceUnpluged(long long socketId);
	//transfer the reply to sending stage
	void moveReplyToSocket(long long socketId, const std::string& reply);

	void onSocketReadable(struct SocketWrapper& socketWrapper);
	void onSocketWritable(struct SocketWrapper& socketWrapper);
	void onSocketError(struct SocketWrapper& socketWrapper);

	void retrieveCommands(std::deque<unsigned char>& data, std::vector<std::string>& commands);
	void onCommand(struct SocketWrapper& socketWrapper, const std::string& command);
	void onCommandDevicesGet(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDevicesGet> cmdPtr);
	void onCommandDeviceConnect(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDeviceConnect> cmdPtr);
	void onCommandDeviceQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDeviceQueryPower> cmdPtr);
	void onCommandBdcsPowerOn(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsPowerOn> cmdPtr);
	void onCommandBdcsPowerOff(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsPowerOff> cmdPtr);
	void onCommandBdcsQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsQueryPower> cmdPtr);
	void onCommandBdcCoast(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcCoast> cmdPtr);
	void onCommandBdcReverse(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcReverse> cmdPtr);
	void onCommandBdcForward(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcForward> cmdPtr);
	void onCommandBdcBreak(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcBreak> cmdPtr);
	void onCommandBdcQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcQuery> cmdPtr);
	void onCommandSteppersPowerOn(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersPowerOn> cmdPtr);
	void onCommandSteppersPowerOff(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersPowerOff> cmdPtr);
	void onCommandSteppersQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersQueryPower> cmdPtr);
	void onCommandStepperQueryResolution(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperQueryResolution> cmdPtr);
	void onCommandStepperConfigStep(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperConfigStep> cmdPtr);
	void onCommandStepperAccelerationBuffer(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperAccelerationBuffer> cmdPtr);
	void onCommandStepperAccelerationBufferDecrement(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperAccelerationBufferDecrement> cmdPtr);
	void onCommandStepperDecelerationBuffer(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperDecelerationBuffer> cmdPtr);
	void onCommandStepperDecelerationBufferIncrement(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperDecelerationBufferIncrement> cmdPtr);
	void onCommandStepperEnable(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperEnable> cmdPtr);
	void onCommandStepperForward(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperForward> cmdPtr);
	void onCommandStepperSteps(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperSteps> cmdPtr);
	void onCommandStepperRun(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperRun> cmdPtr);
	void onCommandStepperConfigHome(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperConfigHome> cmdPtr);
	void onCommandStepperQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperQuery> cmdPtr);
	void onCommandLocatorQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandLocatorQuery> cmdPtr);
	void sendSocketCommandToDevice(long long socketId, const std::string& command);


	long long newSocketId() { return ++_lastSocketId; }

	void pollSockets();
	void cleanupSockets();
};

#endif /* CSOCKETMANAGER_H_ */
