
#include "CDataExchange.h"

void CDataExchange::_initOutputStageAckPacket(unsigned char packetId)
{
	unsigned char crcLow, crcHigh;
	unsigned char * pBuffer = _scsOutputStage.ackPktBuffer;
	
	pBuffer[0] = SCS_ACK_PACKET_TAG;
	pBuffer[1] = packetId;
	_calculateCrc16(pBuffer, 2, &crcLow, &crcHigh);
	pBuffer[2] = crcLow;
	pBuffer[3] = crcHigh;
}

// acknowledge data packet received in input stage
void CDataExchange::_ackInputStageDataPacket(unsigned char packetId) 
{
	switch(_scsOutputStage.state)
	{
		case SCS_OUTPUT_IDLE:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK;
		}
		break;
		case SCS_OUTPUT_SENDING_DATA:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA_PENDING_ACK;
		}
		break;
		case SCS_OUTPUT_WAIT_ACK:
		{
			_initOutputStageAckPacket(packetId);
			_scsOutputStage.ackPktSendingIndex = 0;
			_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK;
		}
		break;
		default:
		{
			//cannot acknowledge Data packet.
		}
		break;
	}
}

// be called when a ACK packet is received in input stage
void CDataExchange::_on_inputStageAckPacketComplete(unsigned char packetId)
{
	_scsOutputStage.ackedDataPktId = packetId;
}

// be called when a Data packet is received in input stage
void CDataExchange::_on_inputStageDataPacketComplete(void)
{
	unsigned char packetId = _scsInputStage.packetBuffer[1];
	
	if(packetId == SCS_INVALID_PACKET_ID) {
		//illegal packet Id.
		printString("ERROR: invalid DATA packet Id\r\n");
		return;
	}
	if(packetId == _scsInputStage.prevDataPktId) {
		//this packet has been received successfully, discard content of this packet.
		_ackInputStageDataPacket(packetId);
		return;
	}
	
	//check continuation of packetId
	if(_scsInputStage.prevDataPktId == SCS_INVALID_PACKET_ID) {
		//this device just starts, any valid packet id is OK
		// do nothing 
	}
	else 
	{
		if(packetId == 0) {
			//this is the first packet from host
			//do nothing
		}
		else 
		{
			unsigned char expectedPacketId = _scsInputStage.prevDataPktId + 1;
				
			if(expectedPacketId == SCS_INVALID_PACKET_ID) {
				expectedPacketId = 1;
			}
			if(packetId != expectedPacketId) {
				printString("ERROR: unexpected host packetId "); 
				printHex(packetId);
				printString(" expect: ");
				printHex(expectedPacketId);
				printString("\r\n");
				return; //ignore this packet.
			}
		}
	}
	
	unsigned char dataLength = _scsInputStage.packetBuffer[2];
	if(dataLength > _getAppInputBufferAvailable()) {
		//not enough capacity in APP's inputBuffer
		//do not send ACK so that this packet is re-sent by host later
		printString("ERROR: not enough APP input buffer\r\n");
		return;
	}

	_writeAppInputBuffer(_scsInputStage.packetBuffer + 3, dataLength);
	_scsInputStage.prevDataPktId = packetId;
	
	_ackInputStageDataPacket(packetId);
}

// check if a packet from host is complete
void CDataExchange::_processScsInputStage(void)
{
	unsigned char c;
	
	if(_scsInputStage.state == SCS_INPUT_IDLE) 
	{
		if(_getChar(&c)) 
		{
			if((c == SCS_DATA_PACKET_TAG) || (c == SCS_ACK_PACKET_TAG))
			{
				_scsInputStage.state = SCS_INPUT_RECEIVING;
				_scsInputStage.timeStamp = counter_get();
				_scsInputStage.packetBuffer[0] = c;
				_scsInputStage.byteAmount = 1;
			}
			else
			{
				//do nothing, ignore this character
			}
		}
	}
	else if(_scsInputStage.state == SCS_INPUT_RECEIVING) 
	{
		if(_getChar(&c)) 
		{
			unsigned char pktType = _scsInputStage.packetBuffer[0];
			
			_scsInputStage.packetBuffer[_scsInputStage.byteAmount] = c;
			_scsInputStage.byteAmount++;
			
			if(pktType == SCS_DATA_PACKET_TAG)
			{
				unsigned char byteAmount = _scsInputStage.byteAmount;
				unsigned char * pBuffer = _scsInputStage.packetBuffer;
				
				if(byteAmount > 3)
				{
					unsigned char dataLength = pBuffer[2];
					
					if(dataLength > SCS_DATA_MAX_LENGTH) 
					{
						printString("ERROR: illegal input data packet length "); printHex(dataLength); printString("\r\n");
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else if(byteAmount == (dataLength + SCS_DATA_PACKET_STAFF_LENGTH))
					{
						// a complete data packet is received
						unsigned char crcLow, crcHigh;
						
						_calculateCrc16(pBuffer, byteAmount - 2, &crcLow, &crcHigh);
						if((crcLow == pBuffer[byteAmount - 2]) && (crcHigh == pBuffer[byteAmount - 1]))
						{
							printString("> D "); printHex(pBuffer[1]); printString("\r\n");
							_on_inputStageDataPacketComplete();							
						}
						else {
							printString("ERROR: corrupted input data packet\r\n");
						}
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else if(byteAmount > SCS_PACKET_MAX_LENGTH)
					{
						//shouldn't occur
						printString("ERROR: data packet overflow: "); printHex(byteAmount); printString("\r\n");
						_scsInputStage.state = SCS_INPUT_IDLE;
					}
					else {
						//do nothing, continue to receive byte.
					}
				}
				else {
					//do nothing, continue to receive byte
				}
			}
			else if(pktType == SCS_ACK_PACKET_TAG)
			{
				if(_scsInputStage.byteAmount == SCS_ACK_PACKET_LENGTH) 
				{
					//a complete ACK packet is received
					unsigned char crcLow, crcHigh;
					
					_calculateCrc16(_scsInputStage.packetBuffer, 2, &crcLow, &crcHigh);
					
					if((crcLow == _scsInputStage.packetBuffer[2]) && (crcHigh == _scsInputStage.packetBuffer[3]))
					{
						unsigned char packetId = _scsInputStage.packetBuffer[1];
						printString("> A "); printHex(packetId); printString("\r\n");
						_on_inputStageAckPacketComplete(packetId);
					}
					else
					{
						printString("ERROR: corrupted input ACK packet\r\n");
					}
					_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				}
				else if(_scsInputStage.byteAmount > SCS_ACK_PACKET_LENGTH) 
				{
					// shouldn't occur
					printString("ERROR: wrong input ACK packet length ");
					printHex(_scsInputStage.byteAmount);
					printString("\r\n");
					_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				}
				else {
					//do nothing, wait until complete packet is received.
				}
			}
			else
			{
				//shouldn't occur
				printString("ERROR: corrupted input stage\r\n");
				_scsInputStage.state = SCS_INPUT_IDLE;
			}
		}
		else
		{
			//check timeout
			if(counter_diff(_scsInputStage.timeStamp) > _scsInputTimeOut) 
			{
				_scsInputStage.state = SCS_INPUT_IDLE; //change to IDLE state.
				printString("ERROR: input stage timed out\r\n");
			}
		}
	}
	else 
	{
		//shouldn't occur
		printString("ERROR: wrong input stage state ");
		printHex(_scsInputStage.state);
		printString("\r\n");
		
		_scsInputStage.state = SCS_INPUT_IDLE;
	}
}

//handle SCS_OUTPUT_IDLE.
// read data to host from APP's outputBuffer.
void CDataExchange::_processScsOutputStageIdle(void)
{
	unsigned char * pPacket = _scsOutputStage.dataPktBuffer;
	unsigned short size = _readOutputBuffer(pPacket + 3, SCS_DATA_MAX_LENGTH);
	unsigned char crcLow, crcHigh;
	
	if(size == 0) {
		return; //no APP data need to be sent to host
	}
	if(size > SCS_DATA_MAX_LENGTH) {
		//shouldn't occur
		printString("ERROR: too much data read from APP's output buffer\r\n");
		return;
	}
	
	//fill the packet staff
	pPacket[0] = SCS_DATA_PACKET_TAG; //tag
	//packet id
	if(_scsOutputStage.currentDataPktId == SCS_INVALID_PACKET_ID) {
		_scsOutputStage.currentDataPktId = 0;
		pPacket[1] = 0;
	}
	else {
		_scsOutputStage.currentDataPktId++;
		if(_scsOutputStage.currentDataPktId == SCS_INVALID_PACKET_ID) {
			_scsOutputStage.currentDataPktId = 1; //turn around
		}
		pPacket[1] = _scsOutputStage.currentDataPktId;
	}
	//length
	pPacket[2] = (unsigned char)size;
	//CRC
	_calculateCrc16(pPacket, size + 3, &crcLow, &crcHigh);
	pPacket[3+size] = crcLow;
	pPacket[4+size] = crcHigh;
	
	_scsOutputStage.dataPktSendingIndex = 0;
	_scsOutputStage.ackedDataPktId = SCS_INVALID_PACKET_ID;
	_scsOutputStage.dataPktTimeStamp = counter_get();
	_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA;	
}

//send out data in output stage as much as possible
void CDataExchange::_processScsOutputStage(void)
{
	switch(_scsOutputStage.state)
	{
		case SCS_OUTPUT_IDLE:
		{
			_processScsOutputStageIdle();
		}
		break;
		case SCS_OUTPUT_SENDING_DATA:
		{
			unsigned char packetLength = _scsOutputStage.dataPktBuffer[2] + SCS_DATA_PACKET_STAFF_LENGTH;
			unsigned char * pStart = _scsOutputStage.dataPktBuffer + _scsOutputStage.dataPktSendingIndex;
			unsigned char remaining = packetLength - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.dataPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//data packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK; 
				printString("< D "); printHex(_scsOutputStage.dataPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much data sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_SENDING_DATA_PENDING_ACK:
		{
			unsigned char packetLength = _scsOutputStage.dataPktBuffer[2] + SCS_DATA_PACKET_STAFF_LENGTH;
			unsigned char * pStart = _scsOutputStage.dataPktBuffer + _scsOutputStage.dataPktSendingIndex;
			unsigned char remaining = packetLength - _scsOutputStage.dataPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.dataPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//data packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK; 
				printString("< D "); printHex(_scsOutputStage.dataPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much data sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_SENDING_ACK_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_WAIT_ACK:
		{
			unsigned char packetId = _scsOutputStage.dataPktBuffer[1];
			
			if(packetId == _scsOutputStage.ackedDataPktId) {
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
			}
			else if(counter_diff(_scsOutputStage.dataPktTimeStamp) > _scsOutputTimeout)
			{
				//time out, send data packet again
				_scsOutputStage.dataPktSendingIndex = 0;
				_scsOutputStage.dataPktTimeStamp = counter_get();
				_scsOutputStage.state = SCS_OUTPUT_SENDING_DATA;
				printString("ERROR: host ACK time out, "); printHex(packetId); printString("\r\n");
			}
		}
		break;
		case SCS_OUTPUT_SENDING_ACK_WAIT_ACK:
		{
			unsigned char * pStart = _scsOutputStage.ackPktBuffer + _scsOutputStage.ackPktSendingIndex;
			unsigned char remaining = SCS_ACK_PACKET_LENGTH - _scsOutputStage.ackPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.ackPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//ACK packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
				printString("< A "); printHex(_scsOutputStage.ackPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much ACK sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_WAIT_ACK;
			}
		}
		break;
		case SCS_OUTPUT_SENDING_ACK:
		{
			unsigned char * pStart = _scsOutputStage.ackPktBuffer + _scsOutputStage.ackPktSendingIndex;
			unsigned char remaining = SCS_ACK_PACKET_LENGTH - _scsOutputStage.ackPktSendingIndex;
			unsigned char size = _putChars(pStart, remaining);
			
			_scsOutputStage.ackPktSendingIndex += size;
			if(size < remaining) {
				//do nothing
			}
			else if(size == remaining) {
				//ACK packet is sent out
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
				printString("< A "); printHex(_scsOutputStage.ackPktBuffer[1]); printString("\r\n");
			}
			else {
				//shouldn't happen
				printString("ERROR: too much ACK sent out\r\n");
				_scsOutputStage.state = SCS_OUTPUT_IDLE;
			}
		}
		break;
		default:
		{
			printString("ERROR: wrong output stage state: "); printHex(_scsOutputStage.state);printString("\r\n");
			_scsOutputStage.state = SCS_OUTPUT_IDLE;
		}
		break;
	}
}

void CDataExchange::Poll(void)
{
	_processScsInputStage();
	_processScsOutputStage();
	_processMonitorStage();
}

void CDataExchange::initScsDataExchange(void)
{
#if DATA_EXCHANGE_THROUGH_USB
	_initUsbInputBuffer();
#endif

	//input stage
	_scsInputTimeOut = SCS_DATA_INPUT_TIMEOUT;
	_scsInputStage.state = SCS_INPUT_IDLE;
	_scsInputStage.prevDataPktId = SCS_INVALID_PACKET_ID;
	
	//output stage
	_scsOutputTimeout = SCS_DATA_OUTPUT_TIMEOUT;
	_scsOutputStage.state = SCS_OUTPUT_IDLE;
	_scsOutputStage.currentDataPktId = SCS_INVALID_PACKET_ID;
	_scsOutputStage.ackedDataPktId = SCS_INVALID_PACKET_ID;
	
	//monitor
	_monitorOutputBufferConsumerIndex = 0;
	_monitorOutputBufferProducerIndex = 0;
}

bool CDataExchange::_writeMonitorChar(unsigned char c)
{
	unsigned short nextProducerIndex = (_monitorOutputBufferProducerIndex + 1) & MONITOR_OUTPUT_BUFFER_LENGTH_MASK;
	if(nextProducerIndex == _monitorOutputBufferConsumerIndex) {
		return false; //buffer full
	}
	_monitorOutputBuffer[_monitorOutputBufferProducerIndex] = c;
	_monitorOutputBufferProducerIndex = nextProducerIndex;
	return true;
}	

void CDataExchange::_processMonitorStage()
{
	if(_monitorOutputBufferConsumerIndex == _monitorOutputBufferProducerIndex) {
		return; // no data to send
	}
	if(_monitorOutputBufferConsumerIndex < _monitorOutputBufferProducerIndex) 
	{
		unsigned char * pBuffer = _monitorOutputBuffer + _monitorOutputBufferConsumerIndex;
		unsigned short size = _monitorOutputBufferProducerIndex - _monitorOutputBufferConsumerIndex;
		unsigned char amount;
		
		if(size > 0xff) {
			size = 0xff;
		}
		amount = _putCharsMonitor(pBuffer, size);			
		_monitorOutputBufferConsumerIndex += amount;
	}
	else 
	{
		// send the first part of data.
		unsigned char * pBuffer = _monitorOutputBuffer + _monitorOutputBufferConsumerIndex;
		unsigned short size = MONITOR_OUTPUT_BUFFER_LENGTH_MASK - _monitorOutputBufferConsumerIndex + 1;
		unsigned char amount;
		
		if(size > 0xff) {
			size = 0xff;
		}
		amount = _putCharsMonitor(pBuffer, size);
		_monitorOutputBufferConsumerIndex = (_monitorOutputBufferConsumerIndex + amount) & MONITOR_OUTPUT_BUFFER_LENGTH_MASK;
	}
}

void CDataExchange::printString(char * pString)
{
	for(; *pString != '\0'; )
	{
		if(_writeMonitorChar(*pString)) {
			pString++;	
		}		
		else {
			break;
		}
	}
}

void CDataExchange::printHex(unsigned char hex)
{
	unsigned char c;
	
	c = (hex >> 4) & 0xf;
	if(c <= 9) {
		_writeMonitorChar(c + '0');
	}
	else {
		_writeMonitorChar(c - 0xA + 'A');
	}
	
	c = hex & 0xf;
	if(c <= 9) {
		_writeMonitorChar(c + '0');
	}
	else {
		_writeMonitorChar(c - 0xA + 'A');
	}
}

void CDataExchange::printChar(unsigned char c)
{
	_writeMonitorChar(c);
}

///// funcitons above are copied from A03_UART ////

CDataExchange::CDataExchange()
{
	initScsDataExchange();
}

unsigned int CDataExchange::SendCommand(unsigned char * pData, unsigned int length)
{
	if(incomingCmdData.size() > 0xFFFF) {
		return 0;
	}

	for(int i=0; i<length; i++) {
		incomingCmdData.push_back(pData[i]);
	}

	return length;
}

void CDataExchange::ClearCommand()
{
	incomingCmdData.clear();
}

void CDataExchange::OnPacketReply(unsigned char * pData, unsigned int length)
{
	for(int i=0; i<length; i++) {
		incomingPacketData.push_back(pData[i]);
	}
}

bool CDataExchange::_calculateCrc16(unsigned char * pData, unsigned char length, unsigned char * pCrcLow, unsigned char * pCrcHigh)
{
	unsigned short crc;

	if((pData == NULL) || (pCrcLow == NULL) || (pCrcHigh == NULL)) {
		printString("ERROR: NULL in _calculateCrc16\r\n");
		return false;
	}

	crc = crc16.GetCRC(pData, length);
	*pCrcLow = crc & 0xff;
	*pCrcHigh = (crc >> 8) & 0xff;

	return true;
}

unsigned char CDataExchange::_putCharsMonitor(unsigned char * pBuffer, unsigned char size)
{
	if(monitorData.size() > 0xffff) {
		return 0;
	}

	for(int i=0; i<size; i++) {
		monitorData.push_back(pBuffer[i]);
	}
	return size;
}

unsigned short CDataExchange::_getAppInputBufferAvailable(void)
{
	return 1024;
}

unsigned short CDataExchange::_writeAppInputBuffer(unsigned char * pBuffer, unsigned short length)
{
	unsigned short amount;

	for(amount = 0; amount < length; amount++) {
		outgoingCmdData.push_back(pBuffer[amount]);
	}

	return length;
}

bool CDataExchange::_getChar(unsigned char * p)
{
	if(p == NULL) {
		return false;
	}
	if(incomingPacketData.empty()) {
		return false;
	}

	*p = incomingPacketData[0];
	incomingPacketData.erase(incomingPacketData.begin());

	return true;
}

bool CDataExchange::_putChar(unsigned char c)
{
	outgoingPacketData.push_back(c);
	return true;
}

unsigned char CDataExchange::_putChars(unsigned char * pBuffer, unsigned char size)
{
	for(int i=0; i<size; i++) {
		outgoingPacketData.push_back(pBuffer[i]);
	}

	return size;
}

unsigned short CDataExchange::counter_get(void)
{
	unsigned short clocks;

	_timeStamp.update();
	clocks = (_timeStamp.epochMicroseconds()/1000) & 0xFFFF;
	return clocks;
}

unsigned short CDataExchange::counter_diff(unsigned short prevCounter)
{
	unsigned short diff;
	unsigned short curClock = counter_get();

	if(curClock >= prevCounter) {
		diff = curClock - prevCounter;
	}
	else {
		diff = 0xFFFF - prevCounter + curClock;
	}

	return diff;
}

 unsigned short CDataExchange::_readOutputBuffer(unsigned char * pBuffer, unsigned short size)
 {
	 unsigned short amount;

	 if(pBuffer == NULL) {
		 printString("ERROR: NULL parameter in _readOutputBuffer\r\n");
		 return 0;
	 }

	 for(amount=0; (amount<size)&&(amount<incomingCmdData.size()); amount++)
	 {
		 pBuffer[amount] = incomingCmdData[amount];
	 }
	 if(amount > 0) {
		 incomingCmdData.erase(incomingCmdData.begin(), incomingCmdData.begin()+amount);
	 }

	 return amount;
 }

 unsigned int CDataExchange::GetCmdReply(unsigned char * pBuffer, unsigned int length)
 {
	 unsigned int amount;

	 for(amount=0; (amount < length) && (amount < outgoingCmdData.size()); amount++) {
		 pBuffer[amount] = outgoingCmdData[amount];
	 }

	 return amount;
 }

 void CDataExchange::ConsumeCmdReply(unsigned int length)
 {
	 if(length > outgoingCmdData.size()) {
		 length = outgoingCmdData.size();
	 }

	 outgoingCmdData.erase(outgoingCmdData.begin(), outgoingCmdData.begin() + length);
 }

 unsigned int CDataExchange::GetPacketData(unsigned char * pBuffer, unsigned int length)
 {
	 unsigned int amount;

	 for(amount=0; (amount < length) && (amount < outgoingPacketData.size()); amount++) {
		 pBuffer[amount] = outgoingPacketData[amount];
	 }

	 return amount;
}

void CDataExchange::ConsumePacketData(unsigned int length)
{
	 if(length > outgoingPacketData.size()) {
		 length = outgoingPacketData.size();
	 }

	 outgoingPacketData.erase(outgoingPacketData.begin(), outgoingPacketData.begin() + length);
}

unsigned int CDataExchange::GetMonitorData(unsigned char * pBuffer, unsigned int length)
{
	 unsigned int amount;

	 for(amount=0; (amount < length) && (amount < monitorData.size()); amount++) {
		 pBuffer[amount] = monitorData[amount];
	 }

	 return amount;
}

void CDataExchange::ConsumeMonitorData(unsigned int length)
{
	 if(length > monitorData.size()) {
		 length = monitorData.size();
	 }

	 monitorData.erase(monitorData.begin(), monitorData.begin() + length);
}

