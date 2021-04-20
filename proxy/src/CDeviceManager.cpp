/*
 * CDeviceManager.cpp
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */
#include "CDeviceManager.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Dynamic/Var.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/JSONException.h"
#include "ProxyLogger.h"

#if defined(_WIN32) || defined(_WIN64)
#include "WinComDevice.h"
#else
#include "LinuxComDevice.h"
#endif

extern ProxyLogger * pLogger;

CDeviceManager::CDeviceManager() : Task("CDeviceManager")
{
	_pObserver = NULL;
}

CDeviceManager::~CDeviceManager() {

}

void CDeviceManager::SetObserver(IDeviceObserver * pObserver)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	_pObserver = pObserver;
}

void CDeviceManager::SendCommand(const std::string& deviceName, const std::string& command)
{
	for(auto it=command.begin(); it!=command.end(); it++) {
		if((*it < ' ') || (*it > '~')) {
			pLogger->LogError("CDeviceManager::SendCommand illegal character in command: " + deviceName + ":" + command);
			return;
		}
	}

	lockMutex("CDeviceManager::SendCommand", "enqueue command: " + deviceName + " : " + command);

	auto it = _devices.begin();
	for(; it != _devices.end(); it++) {
		if(it->state == DeviceState::ACTIVE) {
			if(it->deviceName == deviceName) {
				pLogger->LogDebug("CDeviceManager enqueue command: " + deviceName + ":" + command);
				enqueueCommand(*it, command);
				break;
			}
		}
	}
	if(it == _devices.end()) {
		pLogger->LogError("CDeviceManager::SendCommand failed in sending: " + deviceName + ":" + command);
	}

	unlockMutex();
}

void CDeviceManager::AddDeviceFile(const std::string & deviceFilePath)
{
	Poco::ScopedLock<Poco::Mutex> lock(_mutex);

	bool fileExist = false;

	for(auto it = _deviceFiles.begin(); it != _deviceFiles.end(); it++)
	{
		if(*it == deviceFilePath) {
			fileExist = true;
			break;
		}
	}

	if(!fileExist) {
		_deviceFiles.push_back(deviceFilePath);
	}
}

void CDeviceManager::lockMutex(const std::string & functionName, const std::string & purpose)
{
	do
	{
		if(_mutex.tryLock(MUTEX_TIMEOUT))
		{
			_lockMutexFor = functionName + " " + purpose;
			break;
		}
		else
		{
			pLogger->LogError(functionName + " failed to lock mutex: " + _lockMutexFor);
		}
	} while(1);
}

void CDeviceManager::unlockMutex()
{
	_lockMutexFor.clear();
	_mutex.unlock();
}


void CDeviceManager::onReply(struct Device& device, const std::string& reply)
{
	//convert the reply to a JSON object.
	std::string json = "{" + reply + "}";

	switch(device.state)
	{
		case DeviceState::RECEIVING_NAME:
		{
			if(reply != std::string(COMMAND_QUERY_NAME))
			{
				bool exceptionOccur = false;
				bool nameAvailable = false;
				std::string name;

				try
				{
					Poco::JSON::Parser parser;
					Poco::Dynamic::Var result = parser.parse(json);
					Poco::JSON::Object::Ptr objectPtr = result.extract<Poco::JSON::Object::Ptr>();

					if(objectPtr->has(std::string("name")))
					{
						name = objectPtr->getValue<std::string>("name");
						if(name.size() < 1) {
							pLogger->LogError("CDeviceManager::onReply invalid name in " + reply);
						}
						else {
							nameAvailable = true;
						}
					}
					else
					{
						pLogger->LogError("CDeviceManager::onReply no name in " + reply);
					}
				}
				catch(Poco::JSON::JSONException& e)
				{
					exceptionOccur = true;
					pLogger->LogError("CDeviceManager::onReply exception occurs: " + e.displayText() + " : " + json);
				}
				catch(...)
				{
					exceptionOccur = true;
					pLogger->LogError("CDeviceManager::onReply unknown exception in " + json);
				}

				if(!exceptionOccur && nameAvailable)
				{
					device.deviceName = name;
					device.state = DeviceState::ACTIVE;
					pLogger->LogInfo("CDeviceManager::onReply device inserted: " + device.deviceName + ":" + device.fileName);
					if(_pObserver != nullptr) {
						_pObserver->OnDeviceInserted(device.deviceName);
					}
					else {
						pLogger->LogError("CDeviceManager::onReply invalid observer at receiving name");
					}
				}
				else {
					pLogger->LogError("CDeviceManager::onReply no name in reply");
				}
			}
		}
		break;

		case DeviceState::ACTIVE:
		{
			if(_pObserver != nullptr) {
				_pObserver->OnDeviceReply(device.deviceName, json);
			}
			else {
				pLogger->LogError("CDeviceManager::onReply invalid observer at active");
			}
		}
		break;

		default:
		{
			//do nothing
		}
		break;
	}
}

//read data from device
void CDeviceManager::onDeviceCanBeRead(struct Device& device, std::deque<unsigned char> & reply)
{
	for(; reply.empty() == false; )
	{
		unsigned char c = reply.front();
		device.dataExchange.OnPacketReply(&c, 1);
		reply.pop_front();
	}
}

void CDeviceManager::enqueueCommand(struct Device& device, const char * pCommand)
{
	if(pCommand == nullptr) {
		return;
	}

	std::string command(pCommand);
	enqueueCommand(device, command);
}

void CDeviceManager::enqueueCommand(struct Device& device, const std::string command)
{
	std::vector<unsigned char> array;

	if(command.size() < 1) {
		pLogger->LogError("CDeviceManager::enqueueCommand empty command to device: " + device.fileName);
		return;
	}

	pLogger->LogDebug("CDeviceManager::enqueueCommand enqueue command: " + device.fileName + " : " + command);
	for(auto it=command.begin(); it!=command.end(); it++)
	{
		array.push_back(*it);
	}
	array.push_back(COMMAND_TERMINATER);

	device.dataExchange.SendCmdData(array.data(), array.size());
}

//write a command to device
void CDeviceManager::onDeviceCanBeWritten(struct Device& device, ILowlevelDevice * pLowlevelDevice)
{
	unsigned char buffer[256];
	int size;
	std::vector<unsigned char> data;
	std::string info;

	size = device.dataExchange.GetPacketData(buffer, 256);
	for(int i=0; i<size; i++) {
		data.push_back(buffer[i]);
	}

	if(pLowlevelDevice->SendCommand(data, info) == true) {
		device.dataExchange.ConsumePacketData(size);
	}
	else {
		pLogger->LogError("CDeviceManager::onDeviceCanBeWritten failed to send to: " + device.fileName);
	}
}

void CDeviceManager::onDeviceError(struct Device& device, const std::string & errorInfo)
{
	pLogger->LogError("CDeviceManager device error: " + device.fileName + ", errInfo: " + errorInfo);
	device.state = DeviceState::DEVICE_ERROR;
}

void CDeviceManager::onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info)
{
	bool deviceFound = false;
	pLogger->LogInfo("CDeviceManager::onLowlevelDeviceState device: " + deviceName + " state: " + std::to_string((int)state) + " info: " + info);

	lockMutex("CDeviceManager::onLowlevelDeviceState", deviceName);

	if(state == LowlevelDeviceState::DeviceNormal)
	{
		//check whether this device exists
		for(auto it = _devices.begin(); it != _devices.end(); it++)
		{
			if(deviceName == it->fileName)
			{
				//this device file shouldn't be in _devices before its normal state.
				deviceFound = true;
				pLogger->LogError("CDeviceManager::onLowlevelDeviceState low level device state error: " + deviceName);
				onDeviceError(*it, "low level device error in " + deviceName);
				break;
			}
		}

		if(!deviceFound)
		{
			pLogger->LogInfo("CDeviceManager::onLowlevelDeviceState low level device changes to normal state: " + deviceName);

			Device device;

			device.fileName = deviceName;
			device.state = DeviceState::OPENED;

			_devices.push_back(device);
		}
	}
	else if(state == LowlevelDeviceState::DeviceError)
	{
		//check whether this device exists
		for(auto it = _devices.begin(); it != _devices.end(); it++)
		{
			if(deviceName == it->fileName)
			{
				deviceFound = true;
				onDeviceError(*it, info);
				break;
			}
		}

		if(!deviceFound) {
			pLogger->LogError("CDeviceManager::onLowlevelDeviceState device not found: " + deviceName);
		}
	}

	unlockMutex();
}

void CDeviceManager::onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice)
{
	lockMutex("CDeviceManager::onLowlevelDeviceWritable", deviceName);

	for(auto it = _devices.begin(); it != _devices.end(); it++)
	{
		if(deviceName == it->fileName)
		{
			it->writeStamp.update();
			onDeviceCanBeWritten(*it, pLowlevelDevice);
			break;
		}
	}

	unlockMutex();
}

void CDeviceManager::onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data)
{
	lockMutex("CDeviceManager::onLowlevelDeviceReply", deviceName);

	for(auto it = _devices.begin(); it != _devices.end(); it++)
	{
		if(deviceName == it->fileName)
		{
			onDeviceCanBeRead(*it, data);
			break;
		}
	}

	unlockMutex();
}

void CDeviceManager::pollDevices()
{
	if(_devices.size() < 1) {
		return;
	}

	lockMutex("CDeviceManager::pollDevices", "device polling");

	for(size_t i=0; i<_devices.size(); i++)
	{
		if(_devices[i].writeStamp.elapsed() > _devices[i].FileWriteWarningThreshold)
		{
			_devices[i].writeStamp.update();
			pLogger->LogError("CDeviceManager::pollDevices writing unavailable: " + _devices[i].fileName);
		}
		if(_devices[i].readStamp.elapsed() > _devices[i].FileReadWarningThreshold)
		{
			_devices[i].readStamp.update();
			//pLogger->LogError("CDeviceManager::pollDevices reading unavailable: " + _devices[i].fileName);
		}
	}

	//remove the device having an error
	for(auto deviceIt = _devices.begin(); deviceIt != _devices.end(); )
	{
		if(deviceIt->state == DeviceState::DEVICE_ERROR)
		{
			//notify observers of device's unavailability
			if(!deviceIt->deviceName.empty()) {
				//this device has been fully enumerated.
				_pObserver->OnDeviceUnplugged(deviceIt->deviceName);
			}
			//erase this device
			deviceIt = _devices.erase(deviceIt);
		}
		else
		{
			deviceIt++;
		}
	}

	//poll data exchange
	for(auto deviceIt = _devices.begin(); deviceIt != _devices.end(); )
	{
		unsigned char buffer[64];
		unsigned int size;

		deviceIt->dataExchange.Poll();

		//get reply to command
		for(;;)
		{
			size = deviceIt->dataExchange.GetCmdReply(buffer, 64);
			if(size == 0) {
				break;
			}
			deviceIt->dataExchange.ConsumeCmdReply(size);
			for(int i=0; i<size; i++) {
				deviceIt->reply.push_back(buffer[i]);
			}
		}
		//find a complete reply
		bool replyReady = false;
		for(auto it = deviceIt->reply.begin(); it != deviceIt->reply.end(); it++)
		{
			if((*it == 0x0A) || (*it == 0x0D)) {
				replyReady = true;
				break;
			}
		}
		if(replyReady == false) {
			continue;
		}

		//handle reply.
		std::string reply;
		bool illegal = false;
		for(; deviceIt->reply.size() > 0; )
		{
			unsigned char c = deviceIt->reply.front();
			deviceIt->reply.pop_front(); //delete the first character.

			if((c >= ' ') && (c <= '~')) {
				reply.push_back(c);
			}
			else if((c == 0x0D) || (c == 0x0A)) {
				// a carriage return means that a complete reply is found
				//0x0D is changed to 0x0A in raspberry pi.
				if(illegal) {
					pLogger->LogError("CDeviceManager::pollDevices illegal reply from: " + deviceIt->fileName + " : " + reply);
				}
				else if(!reply.empty()) {
					pLogger->LogInfo("CDeviceManager::pollDevices rely: " + deviceIt->fileName + ":" + reply);
					onReply(*deviceIt, reply);
				}
				break;
			}
			else {
				//illegal character
				if(illegal == false) {
					char tmpBuffer[512];
					sprintf(tmpBuffer, "CDeviceManager::pollDevices illegal character 0x%02x from: %s", c, deviceIt->fileName.c_str());
					pLogger->LogError(tmpBuffer);
					illegal = true;
				}
				reply.push_back(ILLEGAL_CHARACTER_REPLACEMENT);
			}
		}

		//handle device state
		switch(deviceIt->state)
		{
			case DeviceState::OPENED:
			{
				//make device to spit out rubbish in receiving buffer.
				pLogger->LogInfo("CDeviceManager::pollDevices clearing device buffer: " + deviceIt->fileName);
				std::string command;
				command.push_back(COMMAND_TERMINATER);
				enqueueCommand(*deviceIt, command);
				deviceIt->state = DeviceState::CLEARING_BUFFER;
				deviceIt->bufferCleaningStamp.update();
			}
			break;

			case DeviceState::CLEARING_BUFFER:
			{
				if(deviceIt->bufferCleaningStamp.elapsed() > 1000000) {
					//1 second is enough for device to spit out rubbish in receiving buffer.
					deviceIt->state = DeviceState::BUFFER_CLEARED;
					pLogger->LogInfo("CDeviceManager::pollDevices cleared device buffer: " + deviceIt->fileName);
				}
			}
			break;

			case DeviceState::BUFFER_CLEARED:
			{
				//internal command to query device name
				pLogger->LogInfo("CDeviceManager::pollDevices querying device name: " + deviceIt->fileName);
				std::string command(COMMAND_QUERY_NAME);
				enqueueCommand(*deviceIt, command);
				deviceIt->state = DeviceState::RECEIVING_NAME;
			}
			break;

			default:
			{
				//nothing to do
			}
			break;
		}
	}

	unlockMutex();
}

void CDeviceManager::runTask()
{
	pLogger->LogInfo("CDeviceManager::runTask starts");

	//launch low level devices
	{
		std::vector<ILowlevelDevice *> taskVector;
		for(unsigned int i=0; i<_deviceFiles.size(); i++)
		{
			ILowlevelDevice * pLowlevelDevice = nullptr;

#if defined(_WIN32) || defined(_WIN64)
			pLowlevelDevice = new WinComDevice(_deviceFiles[i], this);
#else
			pLowlevelDevice = new LinuxComDevice(_deviceFiles[i], this);
#endif

			if(pLowlevelDevice == nullptr) {
				pLogger->LogError("CDeviceManager::runTask() failed to launch: " + _deviceFiles[i]);
			}
			else {
				taskVector.push_back(pLowlevelDevice);
			}
		}

		for(unsigned int i=0; i<taskVector.size(); i++)
		{
			_tm.start(taskVector[i]);
		}
	}

	while(1)
	{
		if(isCancelled()) {
			break;
		}
		else
		{
			bool noObserver = false;
			{
				lockMutex("CDeviceManager::runTask", "check observer");

				if(_pObserver == NULL) {
					noObserver = true;
				}

				unlockMutex();
			}
			if(noObserver) {
				sleep(10);
				continue;
			}

			pollDevices();
			sleep(1);
		}
	}

	pLogger->LogInfo("CDeviceManager::runTask stopping low level devices...");
	_tm.cancelAll();
	_tm.joinAll();

	pLogger->LogInfo("CDeviceManager::runTask exited");
}




