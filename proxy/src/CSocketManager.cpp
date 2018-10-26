/*
 * CSocketManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include <poll.h>
#include <stddef.h>
#include "Poco/Timespan.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/NetException.h"
#include "CSocketManager.h"
#include "ReplyFactory.h"
#include "ProxyLogger.h"
#include "CommandFactory.h"
#include "ReplyTranslater.h"

extern ProxyLogger * pLogger;

CSocketManager::CSocketManager():Task("SocketManager")
{
	_pDevice = NULL;
	_lastSocketId = STARTING_SOCKET_ID;
}

CSocketManager::~CSocketManager()
{
	// TODO Auto-generated destructor stub
}

void CSocketManager::SetDevice(IDevice * pDevice)
{
	_pDevice = pDevice;
}

void CSocketManager::OnDeviceInserted(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	if(_deviceMap.end() == _deviceMap.find(deviceName)) {
		pLogger->LogInfo("CSocketManager::OnDeviceInserted new device is available: " + deviceName);
		_deviceMap[deviceName].socketId = INVALID_SOCKET_ID;
	}
	else {
		pLogger->LogError("CSocketManager::OnDeviceInserted: device has already existed: " + deviceName);
	}
}

void CSocketManager::OnDeviceUnplugged(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_unpluggedDevices.push_back(deviceName);
}

void CSocketManager::OnDeviceReply(const std::string& deviceName, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceMap.find(deviceName);
	if(_deviceMap.end() != it) {
		pLogger->LogInfo("CDeviceSocketMapping::OnDeviceReply: " + deviceName + ":" + reply);
		it->second.replyPool.push_back(reply);
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceReply: unknown device has a  reply: " + deviceName + ":" + reply);
	}
}

void CSocketManager::onDeviceUnpluged(long long socketId)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _sockets.begin();
	for(; it != _sockets.end(); it++)
	{
		if(it->socketId == socketId) {
			//enqueue event to the outgoing stage.
			auto reply = ReplyFactory::EventDeviceUnplugged();
			for(auto c=reply.begin(); c!=reply.end(); c++) {
				it->outgoing.push_back(*c);
			}
			break;
		}
	}
	if(it == _sockets.end()) {
		pLogger->LogError("CSocketManager::onDeviceUnpluged no socket for socketId is found: " + std::to_string(socketId));
	}
}

void CSocketManager::processUnpluggedDevice()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	for(; _unpluggedDevices.size()>0; )
	{
		//retrieve the first element.
		std::string deviceName = _unpluggedDevices.front();
		_unpluggedDevices.erase(_unpluggedDevices.begin());

		auto it = _deviceMap.find(deviceName);
		if(_deviceMap.end() != it) {
			auto socketId = _deviceMap[deviceName].socketId;

			pLogger->LogInfo("CSocketManager::OnDeviceUnplugged: device is unplugged: " + deviceName);
			_deviceMap.erase(it);

			if(socketId != INVALID_SOCKET_ID) {
				//a socket has connected to this device
				onDeviceUnpluged(socketId);
			}
		}
		else {
			pLogger->LogError("CDeviceSocketMapping::OnDeviceUnplugged: unknown device is unplugged: " + deviceName);
		}
	}

}



void CSocketManager::moveReplyToSocket(long long socketId, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _sockets.begin();
	for(; it!=_sockets.end(); it++)
	{
		if(it->socketId == socketId)
		{
			auto formatedReply = ReplyFactory::Reply(reply);
			//append the reply to buffer
			for(auto c = formatedReply.begin(); c != formatedReply.end(); c++)
			{
				it->outgoing.push_back(*c);
			}
			break;
		}
	}
	if(it == _sockets.end()) {
		pLogger->LogError("CSocketManager::moveReplyToSocket no socket for socketId: " + std::to_string(socketId));
	}
	else {
		pLogger->LogInfo("CSocketManager::moveReplyToSocket send reply: " + std::to_string(socketId) + ":" + reply);
	}
}

void CSocketManager::processReplies()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	for(auto deviceIt = _deviceMap.begin(); deviceIt != _deviceMap.end(); deviceIt++)
	{
		std::string formatedReply;
		auto& deviceData = deviceIt->second;

		if(deviceData.replyPool.empty()) {
			continue;
		}
		if(deviceData.socketId == INVALID_SOCKET_ID) {
			//no socket connects to this device.
			deviceData.replyPool.clear();
			continue;
		}

		for(auto it = deviceData.replyPool.begin(); it != deviceData.replyPool.end(); it++) {
			ReplyTranslater translater(*it);
			std::string jsonReply = translater.ToJsonReply();
			if(jsonReply.empty()) {
				char buf[512];

				sprintf(buf, "CSocketManager::processReplies unknown reply from device %s : %s", deviceIt->first.c_str(), it->c_str());
				pLogger->LogError(buf);
			}
			else {
				moveReplyToSocket(deviceData.socketId, jsonReply);
			}
		}
		deviceData.replyPool.clear();
	}
}

void CSocketManager::AddSocket(StreamSocket& socket)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	struct SocketWrapper wrapper;
	Poco::Timespan zeroSpan;

	try {
		socket.setReceiveTimeout(zeroSpan);
		socket.setSendTimeout(zeroSpan);
	}
	catch(...) {
		pLogger->LogError("CSocketManager::AddSocket: cannot set zero timeout in socket");
	}
	wrapper.socket = socket;
	wrapper.socketId = newSocketId();
	wrapper.state = SocketState::ACTIVE;
	pLogger->LogInfo("CSocketManager::AddSocket socket received: " + socket.peerAddress().toString() + " socketId: " + std::to_string(wrapper.socketId));

	_sockets.push_back(wrapper);
}

//retrieve commands from data
void CSocketManager::retrieveCommands(std::deque<unsigned char>& data, std::vector<std::string>& jsonCommands)
{
	CommandFactory::RetrieveCommand(data, jsonCommands);
}

void CSocketManager::onCommand(struct SocketWrapper& socketWrapper, const std::string& jsonCommand)
{
	pLogger->LogInfo("CSocketManager::onCommand JSON command from socket: " + std::to_string(socketWrapper.socketId) + ": " + jsonCommand);

	CommandTranslator translator(jsonCommand);

	switch(translator.Type())
	{
	case CommandType::DevicesGet:
		onCommandDevicesGet(socketWrapper, translator.GetCommandDevicesGet());
		break;

	case CommandType::DeviceConnect:
		onCommandDeviceConnect(socketWrapper, translator.GetCommandDeviceConnect());
		break;

	case CommandType::DeviceQueryPower:
		onCommandDeviceQueryPower(socketWrapper, translator.GetCommandDeviceQueryPower());
		break;

	case CommandType::BdcsPowerOn:
		onCommandBdcsPowerOn(socketWrapper, translator.GetCommandBdcsPowerOn());
		break;

	case CommandType::BdcsPowerOff:
		onCommandBdcsPowerOff(socketWrapper, translator.GetCommandBdcsPowerOff());
		break;

	case CommandType::BdcsQueryPower:
		onCommandBdcsQueryPower(socketWrapper, translator.GetCommandBdcsQueryPower());
		break;

	case CommandType::BdcCoast:
		onCommandBdcCoast(socketWrapper, translator.GetCommandBdcCoast());
		break;

	case CommandType::BdcReverse:
		onCommandBdcReverse(socketWrapper, translator.GetCommandBdcReverse());
		break;

	case CommandType::BdcForward:
		onCommandBdcForward(socketWrapper, translator.GetCommandBdcForward());
		break;

	case CommandType::BdcBreak:
		onCommandBdcBreak(socketWrapper, translator.GetCommandBdcBreak());
		break;

	case CommandType::BdcQuery:
		onCommandBdcQuery(socketWrapper, translator.GetCommandBdcQuery());
		break;

	case CommandType::SteppersPowerOn:
		onCommandSteppersPowerOn(socketWrapper, translator.GetCommandSteppersPowerOn());
		break;

	case CommandType::SteppersPowerOff:
		onCommandSteppersPowerOff(socketWrapper, translator.GetCommandSteppersPowerOff());
		break;

	case CommandType::SteppersQueryPower:
		onCommandSteppersQueryPower(socketWrapper, translator.GetCommandSteppersQueryPower());
		break;

	case CommandType::StepperQueryResolution:
		onCommandStepperQueryResolution(socketWrapper, translator.GetCommandStepperQueryResolution());
		break;

	case CommandType::StepperConfigStep:
		onCommandStepperConfigStep(socketWrapper, translator.GetCommandStepperConfigStep());
		break;

	case CommandType::StepperAccelerationBuffer:
		onCommandStepperAccelerationBuffer(socketWrapper, translator.GetCommandStepperAccelerationBuffer());
		break;

	case CommandType::StepperAccelerationBufferDecrement:
		onCommandStepperAccelerationBufferDecrement(socketWrapper, translator.GetCommandStepperAccelerationBufferDecrement());
		break;

	case CommandType::StepperDecelerationBuffer:
		onCommandStepperDecelerationBuffer(socketWrapper, translator.GetCommandStepperDecelerationBuffer());
		break;

	case CommandType::StepperDecelerationBufferIncrement:
		onCommandStepperDecelerationBufferIncrement(socketWrapper, translator.GetCommandStepperDecelerationBufferIncrement());
		break;

	case CommandType::StepperEnable:
		onCommandStepperEnable(socketWrapper, translator.GetCommandStepperEnable());
		break;

	case CommandType::StepperForward:
		onCommandStepperForward(socketWrapper, translator.GetCommandStepperForward());
		break;

	case CommandType::StepperSteps:
		onCommandStepperSteps(socketWrapper, translator.GetCommandStepperSteps());
		break;

	case CommandType::StepperRun:
		onCommandStepperRun(socketWrapper, translator.GetCommandStepperRun());
		break;

	case CommandType::StepperConfigHome:
		onCommandStepperConfigHome(socketWrapper, translator.GetCommandStepperConfigHome());
		break;

	case CommandType::StepperQuery:
		onCommandStepperQuery(socketWrapper, translator.GetCommandStepperQuery());
		break;

	case CommandType::LocatorQuery:
		onCommandLocatorQuery(socketWrapper, translator.GetCommandLocatorQuery());
		break;

	case CommandType::Invalid:
		break;

	default:
		break;
	}
}

void CSocketManager::onCommandDevicesGet(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDevicesGet> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	std::vector<std::string> devices;
	for(auto it = _deviceMap.begin(); it!=_deviceMap.end(); it++) {
		devices.push_back(it->first);
	}

	auto package = ReplyFactory::DevicesGet(cmdPtr->CommandId(), devices);
	for(auto it = package.begin(); it!=package.end(); it++) {
		socketWrapper.outgoing.push_back(*it);
	}
}

void CSocketManager::onCommandDeviceConnect(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDeviceConnect> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	bool success = false;
	std::string device = cmdPtr->DeviceName();
	std::string reason;

	auto deviceIt=_deviceMap.begin();
	for(; deviceIt!=_deviceMap.end(); deviceIt++)
	{
		if(deviceIt->first == device)
		{
			//the requested device is found.
			if(deviceIt->second.socketId != INVALID_SOCKET_ID)
			{
				//who is using this device?
				auto socketIt=_sockets.begin();
				for(; socketIt!=_sockets.end(); socketIt++) {
					if(deviceIt->second.socketId == socketIt->socketId) {
						reason = "connected to " + socketIt->socket.peerAddress().toString();
						break;
					}
				}
				if(socketIt == _sockets.end()) {
					//couldn't find the socket
					reason = "internal error";
				}
			}
			else
			{
				deviceIt->second.socketId = socketWrapper.socketId;
				success = true;
			}
			break;
		}
	}
	if(deviceIt == _deviceMap.end()) {
		//cannot find the device
		reason = "cannot find " + device;
	}

	auto package = ReplyFactory::DeviceConnect(cmdPtr->CommandId(), device, success, reason);
	for(auto it = package.begin(); it!=package.end(); it++) {
		socketWrapper.outgoing.push_back(*it);
	}
}

void CSocketManager::sendTranslatedCommandToDevice(long long socketId, const std::string& cmdString)
{
	auto deviceIt = _deviceMap.begin();
	for(; deviceIt!=_deviceMap.end(); deviceIt++)
	{
		if(deviceIt->second.socketId == socketId)
		{
			pLogger->LogInfo(std::string(__FUNCTION__) + " " + cmdString + " >> " + deviceIt->first);
			auto& deviceName = deviceIt->first;
			_pDevice->SendCommand(deviceName, cmdString);
			break;
		}
	}
	if(deviceIt == _deviceMap.end()) {
		pLogger->LogError(std::string(__FUNCTION__) + " socketId hasn't be bonded to device: " + std::to_string(socketId));
	}
}

void CSocketManager::onCommandDeviceQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandDeviceQueryPower> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcsPowerOn(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsPowerOn> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcsPowerOff(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsPowerOff> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcsQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcsQueryPower> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcCoast(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcCoast> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcReverse(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcReverse> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcForward(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcForward> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcBreak(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcBreak> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandBdcQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandBdcQuery> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandSteppersPowerOn(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersPowerOn> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandSteppersPowerOff(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersPowerOff> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandSteppersQueryPower(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandSteppersQueryPower> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperQueryResolution(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperQueryResolution> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperConfigStep(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperConfigStep> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperAccelerationBuffer(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperAccelerationBuffer> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperAccelerationBufferDecrement(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperAccelerationBufferDecrement> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperDecelerationBuffer(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperDecelerationBuffer> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperDecelerationBufferIncrement(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperDecelerationBufferIncrement> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperEnable(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperEnable> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperForward(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperForward> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperSteps(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperSteps> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperRun(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperRun> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperConfigHome(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperConfigHome> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandStepperQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandStepperQuery> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}

void CSocketManager::onCommandLocatorQuery(struct SocketWrapper& socketWrapper, std::shared_ptr<CommandLocatorQuery> cmdPtr)
{
	if(cmdPtr == nullptr) {
		pLogger->LogError(std::string(__FUNCTION__) + " failed in translating JSON");
		return;
	}

	sendTranslatedCommandToDevice(socketWrapper.socketId, cmdPtr->ToString());
}


void CSocketManager::onSocketReadable(struct SocketWrapper& socketWrapper)
{
	const int bufferSize = 1024;
	unsigned char buffer[bufferSize];
	int dataRead;
	Poco::Timespan zeroSpan;

	//read data from socket
	for(;;)
	{
		bool receivingError = true;
		dataRead = 0;
		if(socketWrapper.socket.poll(zeroSpan, Poco::Net::Socket::SELECT_READ))
		{
			try
			{
				dataRead = socketWrapper.socket.receiveBytes(buffer, bufferSize, 0);
				if(dataRead == 0) {
					//if socket can be read but has not data, then peer socket is closed.
					pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketReadable socket closed: %s, socketId: %Ld"),
							socketWrapper.socket.peerAddress().toString(),
							socketWrapper.socketId));

					socketWrapper.state = SocketState::TO_BE_CLOSED;
				}
				else {
					pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketReadable %d bytes from %s, socketId: %Ld"),
							dataRead,
							socketWrapper.socket.peerAddress().toString(),
							socketWrapper.socketId));

					receivingError = false;
					for(int i=0; i<dataRead; i++) {
						socketWrapper.incoming.push_back(buffer[i]); //save data to incoming stage.
					}
				}
			}
			catch(Poco::TimeoutException& e)
			{
				pLogger->LogError(Poco::format(std::string("CSocketManager::onSocketReadable timeout in socket: %Ld"),
						socketWrapper.socketId));
			}
			catch(Poco::Net::NetException& e)
			{
				pLogger->LogError(Poco::format(std::string("CSocketManager::onSocketReadable NetException in socket: %Ld: %s"),
						socketWrapper.socketId,
						e.displayText()));
			}
			catch(...)
			{
				pLogger->LogError(Poco::format(std::string("CSocketManager::onSocketReadable unknown exception in socket: %Ld"),
						socketWrapper.socketId));
			}
		}
		else
		{
			break; //no data to read from socket
		}

		if(receivingError) {
			break;
		}
	}

	if(socketWrapper.state == SocketState::TO_BE_CLOSED) {
		return;
	}
	if(socketWrapper.incoming.size() < 1) {
		return; //no input in incoming stage.
	}

	//parse commands from incoming stage
	std::vector<std::string> commands;
	retrieveCommands(socketWrapper.incoming, commands);
	for(auto it=commands.begin(); it!=commands.end(); it++) {
		onCommand(socketWrapper, *it);
	}
}

void CSocketManager::onSocketWritable(struct SocketWrapper& socketWrapper)
{
	//move pending replies in device wrapper to socket wrapper
	auto it = _deviceMap.begin();
	for(; it!=_deviceMap.end(); it++)
	{
		auto& deviceWrapper = it->second;
		if(socketWrapper.socketId == deviceWrapper.socketId)
		{
			auto& replyPool = deviceWrapper.replyPool;
			for(auto itReply = replyPool.begin(); itReply!=replyPool.end(); itReply++)
			{
				auto reply = ReplyFactory::Reply(*itReply);
				for(auto c=reply.begin(); c!=reply.end(); c++) {
					socketWrapper.outgoing.push_back(*c);
				}
			}
			replyPool.clear();
			break;
		}
	}

	if(socketWrapper.outgoing.size() < 1) {
		return; //no data to write to socket
	}
	{
		const int bufferSize = 1024;
		unsigned char buffer[bufferSize];
		int dataSize;
		int dataSent;
		Poco::Timespan zeroSpan;

		for(;;)
		{
			if(!socketWrapper.socket.poll(zeroSpan, Poco::Net::Socket::SELECT_WRITE)) {
				break;
			}
			//fill buffer
			dataSize = 0;
			for(auto it=socketWrapper.outgoing.begin(); it!=socketWrapper.outgoing.end(); it++) {
				buffer[dataSize] = *it;
				dataSize++;
				if(dataSize >= bufferSize) {
					break;
				}
			}
			//write to socket
			dataSent = 0;
			if(dataSize > 0) {
				pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketWritable writing %d bytes to socket %Ld"), dataSize, socketWrapper.socketId));
				dataSent = socketWrapper.socket.sendBytes(buffer, dataSize, 0);
			}
			else {
				break;//no data to send
			}
			//remove the sent data from the sending stage.
			if(dataSent > 0) {
				pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketWritable wrote %d bytes to socket %Ld"), dataSize, socketWrapper.socketId));
				for(;dataSent>0; dataSent--) {
					socketWrapper.outgoing.pop_front();
				}
			}
			else {
				pLogger->LogError(Poco::format(std::string("CSocketManager::onSocketWritable wrote no byte to socket %Ld"), socketWrapper.socketId));
				break;
			}
		}
	}
}

void CSocketManager::onSocketError(struct SocketWrapper& socketWrapper)
{
	pLogger->LogError("CSocketManager::onSocketError socket to be closed: " + std::to_string(socketWrapper.socketId));
	socketWrapper.state = SocketState::TO_BE_CLOSED;
}

void CSocketManager::cleanupSockets()
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);
	for(auto it=_sockets.begin(); it!=_sockets.end(); )
	{
		if(it->state == SocketState::TO_BE_CLOSED)
		{
			for(auto deviceIt=_deviceMap.begin(); deviceIt!=_deviceMap.end(); deviceIt++) {
				if(it->socketId == deviceIt->second.socketId) {
					pLogger->LogInfo("CSocketManager::cleanupSockets socket disconnects from device: " + std::to_string(it->socketId) + ":" + deviceIt->first);
					deviceIt->second.socketId = INVALID_SOCKET_ID;
				}
			}

			try {
				it->socket.close();
			}
			catch(Poco::Exception& e) {
				pLogger->LogError("CSocketManager::cleanupSockets exception occurs: " + std::to_string(it->socketId) + " " + e.displayText());
			}
			catch(...) {
				pLogger->LogError("CSocketManager::cleanupSockets exception occurs: " + std::to_string(it->socketId));
			}
			auto socketId = it->socketId;
			it = _sockets.erase(it);
			pLogger->LogInfo("CSocketManager::cleanupSockets socket is deleted: " + std::to_string(socketId));
		}
		else {
			it++;
		}
	}
}

void CSocketManager::pollSockets()
{
	if(_sockets.size() < 1) {
		return;
	}

	using Poco::Net::Socket;
	using Poco::Timespan;

	std::vector<struct pollfd> fdVector;
	Socket::SocketList readList, writeList, exceptionList;
	Timespan zeroSpan;
	Timespan timedSpan(10*1000); //10 ms
	int socketAmount;

	//check if socket can accept replies/events.
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);
		for(auto it = _sockets.begin(); it != _sockets.end(); it++)
		{
			writeList.push_back(it->socket);
			exceptionList.push_back(it->socket);
		}
	}
	try {
		socketAmount = 0;
		socketAmount = Socket::select(readList, writeList, exceptionList, zeroSpan);
	}
	catch(Poco::TimeoutException& e) {
		pLogger->LogError("CSocketManager::pollSockets timeout exception in select: " + e.displayText());
	}
	catch(Poco::Exception& e) {
		pLogger->LogError("CSocketManager::pollSockets exception in select: " + e.displayText());
	}
	catch(...) {
		pLogger->LogError("CSocketManager::pollSockets unknown exception in select");
	}
	if(socketAmount > 0)
	{
		//process exception list
		for(auto it = exceptionList.begin(); it != exceptionList.end(); it++)
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);
			for(auto wrapperIt = _sockets.begin(); wrapperIt != _sockets.end(); wrapperIt++) {
				if(*it == wrapperIt->socket) {
					onSocketError(*wrapperIt);
				}
			}
		}
		cleanupSockets();

		//process writable list
		for(auto it = writeList.begin(); it != writeList.end(); it++)
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);
			for(auto wrapperIt = _sockets.begin(); wrapperIt != _sockets.end(); wrapperIt++) {
				if(*it == wrapperIt->socket) {
					onSocketWritable(*wrapperIt);
				}
			}
		}
		cleanupSockets();
	}

	//clear lists
	readList.clear();
	writeList.clear();
	exceptionList.clear();

	//check if socket has new commands.
	{
		Poco::ScopedLock<Poco::Mutex> lock(_mutex);
		for(auto it = _sockets.begin(); it != _sockets.end(); it++)
		{
			readList.push_back(it->socket);
			exceptionList.push_back(it->socket);
		}
	}
	try {
		socketAmount = 0;
		socketAmount = Socket::select(readList, writeList, exceptionList, timedSpan);
	}
	catch(Poco::TimeoutException& e) {
		pLogger->LogError("CSocketManager::pollSockets timeout exception in select: " + e.displayText());
	}
	catch(Poco::Exception& e) {
		pLogger->LogError("CSocketManager::pollSockets exception in select: " + e.displayText());
	}
	catch(...) {
		pLogger->LogError("CSocketManager::pollSockets unknown exception in select");
	}
	if(socketAmount > 0)
	{
		//process exception list
		for(auto it = exceptionList.begin(); it != exceptionList.end(); it++)
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);
			for(auto wrapperIt = _sockets.begin(); wrapperIt != _sockets.end(); wrapperIt++) {
				if(*it == wrapperIt->socket) {
					onSocketError(*wrapperIt);
				}
			}
		}
		cleanupSockets();

		//process readable list
		for(auto it = readList.begin(); it != readList.end(); it++)
		{
			Poco::ScopedLock<Poco::Mutex> lock(_mutex);
			for(auto wrapperIt = _sockets.begin(); wrapperIt != _sockets.end(); wrapperIt++) {
				if(*it == wrapperIt->socket) {
					onSocketReadable(*wrapperIt);
				}
			}
		}
		cleanupSockets();
	}
}

void CSocketManager::runTask()
{
	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			if(_sockets.size() < 1) {
				sleep(100);
			}

			processUnpluggedDevice();

			processReplies();

			pollSockets();
		}
	}

	pLogger->LogInfo("CSocketManager::runTask exited");
}

