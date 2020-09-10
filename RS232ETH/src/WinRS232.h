#pragma once

#if defined(_WIN32) || defined(_WIN64)

#include <string>
#include <deque>
#include <vector>

#include "Poco/Task.h"

#include "IDataExchange.h"

enum class DeviceState
{
    DeviceConnected,
    DeviceNormal,
    DeviceError,
    DeviceNotConnected
};

class WinRS232: public IDataExchange, public Poco::Task
{
public:
    WinRS232(const std::string devicePath);
    ~WinRS232();
    void Send(const unsigned char * pData, const unsigned int amount) override;
    void Connect(IDataExchange * pInterface) override;
    void runTask() override;

private:
    std::string _name;
    IDataExchange * _pPeer;

    static const unsigned int MAX_BUFFER_SIZE = 0x10000;
    const unsigned int READING_TIMEOUT = 10; // 10 milliseconds
    const unsigned int WRITING_TIMEOUT = 10; // 10 milliseconds

    Poco::Mutex _mutex;

    HANDLE _handle;;
    bool _bExit;
    DeviceState _state;

    HANDLE _writeEventHandle;
    HANDLE _readEventHandle;
    bool _writeFinished;
    bool _readFinished;
    OVERLAPPED _writeOverlap;
    OVERLAPPED _readOverlap;

    unsigned char _inputBuffer[MAX_BUFFER_SIZE];
    unsigned char _outputBuffer[MAX_BUFFER_SIZE];
    std::deque<unsigned char> _inputQueue;
    std::deque<unsigned char> _outputQueue;

    void openDevice();
    void processReceivedData();
    void readRS232();
};











#endif
