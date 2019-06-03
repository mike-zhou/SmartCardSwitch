/*
 * CDeviceManager.h
 *
 *  Created on: Sep 17, 2018
 *      Author: user1
 */

#ifndef CDEVICEMANAGER_H_
#define CDEVICEMANAGER_H_

#include <map>
#include <vector>
#include <deque>
#include <string>
#include "Poco/Task.h"
#include "Poco/TaskManager.h"
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"
#include "IDevice.h"
#include "IDeviceObserver.h"
#include "ILowlevelDevice.h"
#include "CrcCcitt.h"


/***************
 * Functionalities of this class object
 * 1. enumerate devices
 * 2. send command to devices
 * 3. notify observer/s of replies/events from device.
  ****************/
class CDeviceManager: public Poco::Task, public IDevice, public ILowlevelDeviceObsesrver
{
public:
	CDeviceManager();
	virtual ~CDeviceManager();

	void SetObserver(IDeviceObserver * pObserver);
	void StartMonitoringDevices();
	void AddDeviceFile(const std::string & deviceFilePath);

private:
	// Called by DeviceSocketMapping object to send a command to device.
	virtual void SendCommand(const std::string& deviceName, const std::string& command) override;

	virtual void onLowlevelDeviceState(const std::string & deviceName, const LowlevelDeviceState state, const std::string & info) override;
	virtual void onLowlevelDeviceWritable(const std::string & deviceName, ILowlevelDevice * pLowlevelDevice) override;
	virtual void onLowlevelDeviceReply(const std::string & deviceName, std::deque<unsigned char> & data) override;

	void runTask();

private:
	Poco::Mutex _mutex;

	const char ILLEGAL_CHARACTER_REPLACEMENT = '?';
	const char * COMMAND_QUERY_NAME = "C 1 0";
	const char COMMAND_TERMINATER = 0x0D; //carriage return

	//definitions for data exchange
	static const int PACKET_SIZE  = 64;
	static const unsigned char DATA_PACKET_TAG = 0xDD;
	static const unsigned char ACK_PACKET_TAG = 0xAA;
	const long DATA_INPUT_TIMEOUT = 50000; //50,000 microseconds
	const long DATA_ACK_TIMEOUT = 200000; //200,000 microseconds
	static const unsigned char INVALID_PACKET_ID = 0xFF;
	static const unsigned char INITAL_PACKET_ID = 0; // this id is used only for the first packet after app starts.

	bool _startMonitoringDevices;

	enum InputStageState
	{
		INPUT_RECEIVING = 0,
		INPUT_ACKNOWLEDGING
	};

	struct DataInputStage
	{
		unsigned char buffer[PACKET_SIZE];
		InputStageState state;
		unsigned int byteAmount;
		Poco::Timestamp inputTimeStamp;
		unsigned char previousId;
		CrcCcitt crc16;

		DataInputStage();
	};

	enum OutputStageState
	{
		OUTPUT_IDLE = 0, // ready for packet sending
		OUTPUT_SENDING, // is sending a packet
		OUTPUT_WAITING_ACK, // is waiting for acknowledgment
		OUTPUT_WAITING_ACK_WHILE_SENDING // is waiting for acknowledgment, and is sending an acknowledgment
	};

	struct DataOutputStage
	{
		DataOutputStage();
		void IncreasePacketId();
		void OnAcknowledgment(unsigned char packetId);
		//return true if ACK packet can be sent
		//return false if ACK packet cannot be sent
		bool SendAcknowledgment(unsigned char packetId);
		//if possible, pop data from queue and sent it.
		void SendData(std::deque<char>& dataQueue);

		unsigned char packet[PACKET_SIZE];
		unsigned char buffer[PACKET_SIZE];
		OutputStageState state;
		unsigned int sendingIndex;
		Poco::Timestamp ackTimeStamp;
		unsigned char packetId;

		CrcCcitt crc16;
	};

	struct DataExchange
	{
		DataInputStage inputStage;
		DataOutputStage outputStage;
	};

	enum DeviceState
	{
		CLOSED = 0,
		OPENED,
		CLEARING_BUFFER,
		BUFFER_CLEARED,
		RECEIVING_NAME,
		ACTIVE,
		DEVICE_ERROR
	};

	std::vector<std::string> _deviceFiles;

	struct Device
	{
		static const Poco::Timestamp::TimeDiff FileReadWarningThreshold = 1000000; //1 second
		static const Poco::Timestamp::TimeDiff FileWriteWarningThreshold = 1000000; // 1 second

		enum DeviceState state;
		Poco::Timestamp bufferCleaningStamp;
		Poco::Timestamp readStamp;
		Poco::Timestamp writeStamp;

		std::string fileName; //name of device file
		std::string deviceName; //name queried from COMMAND_QUERY_NAME. Each device is supposed to have a unique name.
		std::deque<char> outgoing;
		std::deque<char> incoming;

		DataExchange dataExchange;
	};

	std::vector<struct Device> _devices;

	void onReply(struct Device& device, const std::string& reply);
	void onDeviceCanBeRead(struct Device& device, std::deque<unsigned char> & reply);
	void onDeviceCanBeWritten(struct Device& device, ILowlevelDevice * pLowlevelDevice);
	void onDeviceError(struct Device& device, const std::string & errorInfo);
	void pollDevices();

	void enqueueCommand(struct Device& device, const char * pCommand);
	void enqueueCommand(struct Device& device, const std::string command);

	IDeviceObserver * _pObserver;
	Poco::TaskManager _tm;
};

#endif /* CDEVICEMANAGER_H_ */
