/*
 * CSocketManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#include "CSocketManager.h"
#include "ProxyLogger.h"
#include "ReplyFactory.h"
#include <poll.h>
#include <stddef.h>
#include "Poco/Net/Socket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Timespan.h"


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

	if(_deviceSocketMap.end() == _deviceSocketMap.find(deviceName)) {
		pLogger->LogInfo("CSocketManager::OnDeviceInserted new device is available: " + deviceName);
		_deviceSocketMap[deviceName].socketId = INVALID_SOCKET_ID;
	}
	else {
		pLogger->LogError("CSocketManager::OnDeviceInserted: device has already existed: " + deviceName);
	}
}

void CSocketManager::OnDeviceUnplugged(const std::string& deviceName)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
		auto socketId = _deviceSocketMap[deviceName].socketId;

		pLogger->LogInfo("CSocketManager::OnDeviceUnplugged: device is unplugged: " + deviceName);
		_deviceSocketMap.erase(it);

		if(socketId != INVALID_SOCKET_ID) {
			onDeviceUnpluged(socketId);
		}
	}
	else {
		pLogger->LogError("CDeviceSocketMapping::OnDeviceUnplugged: unknown device is unplugged: " + deviceName);
	}
}

void CSocketManager::OnDeviceReply(const std::string& deviceName, const std::string& reply)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	auto it = _deviceSocketMap.find(deviceName);
	if(_deviceSocketMap.end() != it) {
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
		pLogger->LogError("CSocketManager::onDeviceReplyAvailable no socket for socketId: " + std::to_string(socketId));
	}
	else {
		pLogger->LogInfo("CSocketManager::onDeviceReplyAvailable send reply: " + std::to_string(socketId) + ":" + reply);
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

	_sockets.push_back(wrapper);
}

void CSocketManager::onSocketReadable(struct SocketWrapper& socketWrapper)
{
	const int bufferSize = 1024;
	unsigned char buffer[bufferSize];
	int dataRead;

}

void CSocketManager::onSocketWritable(struct SocketWrapper& socketWrapper)
{
	//move pending replies in device wrapper to socket wrapper
	auto it = _deviceSocketMap.begin();
	for(; it!=_deviceSocketMap.end(); it++)
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
				pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketWritable write %d bytes to socket %d"), dataSize, socketWrapper.socketId));
				dataSent = socketWrapper.socket.sendBytes(buffer, dataSize, 0);
			}
			else {
				break;//no data to send
			}
			//remove the sent data from the sending stage.
			if(dataSent > 0) {
				pLogger->LogInfo(Poco::format(std::string("CSocketManager::onSocketWritable wrote %d bytes to socket %d"), dataSize, socketWrapper.socketId));
				for(;dataSent>0; dataSent--) {
					socketWrapper.outgoing.pop_front();
				}
			}
			else {
				pLogger->LogError(Poco::format(std::string("CSocketManager::onSocketWritable wrote no byte to socket %d"), socketWrapper.socketId));
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

	//poll devices output.
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

	//poll devices input.
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
			else {
				pollSockets();
			}
		}
	}
}

