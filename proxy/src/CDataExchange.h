#ifndef _CDATAEXCHANGE_H_
#define _CDATAEXCHANGE_H_

#include <vector>
#include "Poco/Mutex.h"
#include "Poco/Timestamp.h"

#include "CrcCcitt.h"

/**
 * this class accept a block of Cmd data, divides it to different packets, then sends out packets one by one.
 * it also merges replied packets to a data block.
 * Poll() needs to be called repeatedly to drive this class
 */
class CDataExchange
{
public:
    CDataExchange();
    void Poll();

    /**
     * Send a command string to the device.
     * Parameters:
     * 		pData: address of the command
     * 		length: length of command
     * Return value:
     * 		amount of bytes accepted.
     */
    unsigned int SendCommand(unsigned char * pData, unsigned int length);
    /**
     * delete any content which hasn't beeen sent.
     */
    void ClearCommand();
    /**
     * read reply
     * Parameters:
     * 		pBuffer: address where the reply can be written to
     * 		length: length of buffer
     * Return value:
     * 		amount of data written to pBuffer
     */
    unsigned int GetCmdReply(unsigned char * pBuffer, unsigned int length);
    /**
     * Notify how much reply is consumed
     */
    void ConsumeCmdReply(unsigned int length);

    void OnPacketReply(unsigned char * pData, unsigned int length);
    unsigned int GetPacketData(unsigned char * pBuffer, unsigned int length);
    void ConsumePacketData(unsigned int length);

    unsigned int GetMonitorData(unsigned char * pBuffer, unsigned int length);
    void ConsumeMonitorData(unsigned int length);

private:
    /*********************************************************
    * Data exchange stages
    **********************************************************/
    #define SCS_PACKET_MAX_LENGTH 64
    #define SCS_DATA_PACKET_TAG 0xDD
    /************************************************************************/
    /* 
    Data packet structure:
        0xDD		// 1 byte
        id			// 1 byte: 0, 1 to 0xFE
        dataLength	// 1 byte
        data bytes	// bytes of dataLength
        crcLow		// 1 byte
        crcHigh		// 1 byte                                                                    
    */
    /************************************************************************/
    #define SCS_ACK_PACKET_TAG 0xAA
    /************************************************************************/
    /*          
    Acknowledge packet structure:
        0xAA		// 1 byte
        id			// 1 byte: 0, 1 to 0xFE
        crcLow      // 1 byte
        crcHigh		// 1 byte                                           
    */
    /************************************************************************/
    #define SCS_DATA_PACKET_STAFF_LENGTH 5 //tag, id, dataLength, crcLow, crcHigh
    #define SCS_DATA_MAX_LENGTH (SCS_PACKET_MAX_LENGTH - SCS_DATA_PACKET_STAFF_LENGTH)
    #define SCS_ACK_PACKET_LENGTH 4
    #define SCS_DATA_INPUT_TIMEOUT 50 //milliseconds
    #define SCS_DATA_OUTPUT_TIMEOUT 200 //milliseconds
    #define SCS_INITIAL_PACKET_ID 0 //this id is used only once at the launch of application
    #define SCS_INVALID_PACKET_ID 0xFF

    enum SCS_Input_Stage_State
    {
        SCS_INPUT_IDLE = 0,
        SCS_INPUT_RECEIVING
    };

    struct SCS_Input_Stage
    {
        enum SCS_Input_Stage_State state;
        unsigned char packetBuffer[SCS_PACKET_MAX_LENGTH];
        unsigned char byteAmount;
        unsigned short timeStamp;
        unsigned char prevDataPktId;
    };

    enum SCS_Output_Stage_State
    {
        SCS_OUTPUT_IDLE = 0,
        SCS_OUTPUT_SENDING_DATA,
        SCS_OUTPUT_SENDING_ACK,
        SCS_OUTPUT_SENDING_DATA_PENDING_ACK,
        SCS_OUTPUT_WAIT_ACK,
        SCS_OUTPUT_SENDING_ACK_WAIT_ACK
    };

    struct SCS_Output_Stage
    {
        enum SCS_Output_Stage_State state;
        //data packet
        unsigned char dataPktBuffer[SCS_PACKET_MAX_LENGTH];
        unsigned char dataPktSendingIndex; //index of byte to be sent
        unsigned char currentDataPktId;
        unsigned char ackedDataPktId;
        unsigned short dataPktTimeStamp;
        //acknowledge packet
        unsigned char ackPktBuffer[SCS_ACK_PACKET_LENGTH];
        unsigned char ackPktSendingIndex; //index of byte to be sent
    };

    SCS_Input_Stage _scsInputStage;
    unsigned short _scsInputTimeOut;
    SCS_Output_Stage _scsOutputStage;
    unsigned short _scsOutputTimeout;

    #define MONITOR_OUTPUT_BUFFER_LENGTH_MASK 0xFF
    unsigned char _monitorOutputBuffer[MONITOR_OUTPUT_BUFFER_LENGTH_MASK + 1];
    unsigned short _monitorOutputBufferConsumerIndex;
    unsigned short _monitorOutputBufferProducerIndex;

    bool _writeMonitorChar(unsigned char c);

    void _initOutputStageAckPacket(unsigned char packetId);
    void _ackInputStageDataPacket(unsigned char packetId);
    void _on_inputStageAckPacketComplete(unsigned char packetId);
    void _on_inputStageDataPacketComplete(void);
    void _processScsOutputStageIdle(void);

	void _processScsInputStage();
	void _processScsOutputStage();
	void _processMonitorStage();

    void printString(char * pString);
    void printHex(unsigned char hex);
    void printChar(unsigned char c);
    void initScsDataExchange(void);

    //// definitions, variables and functions above are about packetlization and reliable exchange, and are copied from A03_UART ///////////

    // the following functions support 
    bool _calculateCrc16(unsigned char * pData, unsigned char length, unsigned char * pCrcLow, unsigned char * pCrcHigh);
    unsigned char _putCharsMonitor(unsigned char * pBuffer, unsigned char size);
    unsigned short _getAppInputBufferAvailable(void);
    unsigned short _writeAppInputBuffer(unsigned char * pBuffer, unsigned short length);
    bool _getChar(unsigned char * p);
    bool _putChar(unsigned char c);
    unsigned char _putChars(unsigned char * pBuffer, unsigned char size);
    unsigned short counter_get(void);
    unsigned short counter_diff(unsigned short prevCounter);
    unsigned short _readOutputBuffer(unsigned char * pBuffer, unsigned short size);
    Poco::Timestamp _timeStamp;
    CrcCcitt crc16;
    std::vector<unsigned char> incomingCmdData;
    std::vector<unsigned char> outgoingCmdData;
    std::vector<unsigned char> incomingPacketData;
    std::vector<unsigned char> outgoingPacketData;
    std::vector<unsigned char> monitorData;
};



#endif
