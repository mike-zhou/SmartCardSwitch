#if defined(_WIN32) || defined(_WIN64)

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "Poco/File.h"

#include "WinRS232.h"
#include "Logger.h"

extern Logger * pLogger;


WinRS232::WinRS232(const std::string devicePath) : Task("RS232")
{
    _state = DeviceState::DeviceNotConnected;
    _name = devicePath;
    _pPeer = nullptr;
    _handle = INVALID_HANDLE_VALUE;
    _bExit = false;
    _nullWritingHappened = false;
    _readEventHandle = CreateEventA(NULL, TRUE, FALSE, NULL);
    _writeEventHandle = CreateEventA(NULL, TRUE, FALSE, NULL);
    _totalRead = 0;
    _totalWrite = 0;
}

WinRS232::~WinRS232()
{

}

void WinRS232::openDevice()
{
    std::string modeStr;
    COMMTIMEOUTS Cptimeouts;
    DCB port_settings;

    _handle = CreateFileA(_name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,                          /* no share  */
        NULL,                       /* no security */
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,                     
        NULL);                      /* no templates */
    if (_handle == INVALID_HANDLE_VALUE) {
        pLogger->LogError("WinRS232::openDevice failed to open: " + _name);
        return;
    }

    //if (!SetCommMask(_handle, EV_RXCHAR | EV_TXEMPTY)) {
    //    pLogger->LogError("WinRS232::openDevice failed to set events to comm file");
    //    return;
    //}

    modeStr = "baud=115200 data=8 parity=E stop=1 dtr=off rts=off";
    memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
    port_settings.DCBlength = sizeof(port_settings);
    if (!BuildCommDCBA(modeStr.c_str(), &port_settings))
    {
        pLogger->LogError("WinRS232::openDevice unable to set comport dcb settings");
        CloseHandle(_handle);
        return;
    }
    if (!SetCommState(_handle, &port_settings))
    {
        pLogger->LogError("WinRS232::openDevice unable to set comport cfg settings");
        CloseHandle(_handle);
        return;
    }

    Cptimeouts.ReadIntervalTimeout = READING_TIMEOUT;
    Cptimeouts.ReadTotalTimeoutMultiplier = 0;
    Cptimeouts.ReadTotalTimeoutConstant = READING_TIMEOUT;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant = WRITING_TIMEOUT;
    if (!SetCommTimeouts(_handle, &Cptimeouts))
    {
        pLogger->LogError("WinRS232::openDevice unable to set comport time-out settings");
        CloseHandle(_handle);
        return;
    }

    _writeFinished = true;
    _readFinished = true;
    _writeOverlap = { 0 };
    _readOverlap = { 0 };
    _writeOverlap.hEvent = _writeEventHandle; //manual reset
    _readOverlap.hEvent = _readEventHandle;

    _state = DeviceState::DeviceNormal;

    pLogger->LogInfo("WinRS232::openDevice file is opened: " + _name);
}

void WinRS232::readWriteRS232()
{
    if (_readFinished) 
    {
        //read serial port
        DWORD bytesRead;
        _readOverlap = { 0 };
        _readOverlap.hEvent = _readEventHandle;
        ResetEvent(_readEventHandle);

        BOOL rc = ReadFile(_handle, _inputBuffer, MAX_BUFFER_SIZE, &bytesRead, &_readOverlap);
        if (rc)
        {
            char buf[16];
            std::string content;

            for (int i = 0; i < bytesRead; i++) {
                _inputQueue.push_back(_inputBuffer[i]);
                sprintf(buf, "%02x ", _inputBuffer[i]);
                content.push_back(buf[0]);
                content.push_back(buf[1]);
                content.push_back(buf[2]);
            }
            _totalRead += bytesRead;
            pLogger->LogInfo("WinRS232::readWriteRS232 read com bytes directly: " + std::to_string(bytesRead) + ", totally read: " + std::to_string(_totalRead));
            pLogger->LogInfo("WinRS232::readWriteRS232 read content 0x: " + content);
        }
        else
        {
            auto err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                _readFinished = false;
            }
            else
            {
                pLogger->LogError("WinRS232::readWriteRS232 failed to read com: " + std::to_string(err));
                _state = DeviceState::DeviceError;
                return;
            }
        }
    }

    if (_writeFinished) 
    {
        //write serial port
        Poco::ScopedLock<Poco::Mutex> lock(_mutex);

        if (!_outputQueue.empty())
        {
            DWORD bytesWritten;
            DWORD bytesToWrite;

            _writeOverlap = { 0 };
            _writeOverlap.hEvent = _writeEventHandle;
            ResetEvent(_writeEventHandle);

            for (bytesToWrite = 0; (bytesToWrite < _outputQueue.size()) && (bytesToWrite < MAX_BUFFER_SIZE); bytesToWrite++) {
                _outputBuffer[bytesToWrite] = _outputQueue[bytesToWrite];
            }

            auto rc = WriteFile(_handle, _outputBuffer, bytesToWrite, &bytesWritten, &_writeOverlap);
            if (rc)
            {
                if (bytesWritten > 0) 
                {
                    char buf[16];
                    std::string content;

                    for (int i = 0; i < bytesWritten; i++) {
                        sprintf(buf, "%02x ", _outputBuffer[i]);
                        content.push_back(buf[0]);
                        content.push_back(buf[1]);
                        content.push_back(buf[2]);
                    }

                    _nullWritingHappened = false;
                    _outputQueue.erase(_outputQueue.begin(), _outputQueue.begin() + bytesWritten);
                    _totalWrite += bytesWritten;
                    pLogger->LogInfo("WinRS232::readWriteRS232 wrote com bytes directly: " + std::to_string(bytesWritten) + ", totally write: " + std::to_string(_totalWrite));
                    pLogger->LogInfo("WinRS232::readWriteRS232 wrote content 0x: " + content);
                }
                else 
                {
                    if (_nullWritingHappened == false) {
                        _nullWritingHappened = true;
                        pLogger->LogError("WinRS232::readWriteRS232 null writing com happened: " + std::to_string(bytesToWrite));
                    }
                    sleep(1);
                }
            }
            else
            {
                auto err = GetLastError();
                if (err == ERROR_IO_PENDING) {
                    _writeFinished = false;
                    pLogger->LogInfo("WinRS232::readWriteRS232 pending write com bytes: " + std::to_string(bytesToWrite));
                }
                else
                {
                    pLogger->LogError("WinRS232::readWriteRS232 failed to write com: " + std::to_string(err));
                    _state = DeviceState::DeviceError;
                    return;
                }
            }
        }
    }

    if ((!_readFinished) || (!_writeFinished))
    {
        HANDLE handles[2];

        handles[0] = _readOverlap.hEvent;
        handles[1] = _writeOverlap.hEvent;

        auto rc = WaitForMultipleObjects(2, handles, FALSE, READING_TIMEOUT);
        switch (rc)
        {
            case WAIT_OBJECT_0:
            {
                DWORD bytesRead;
                if (GetOverlappedResult(_handle, &_readOverlap, &bytesRead, FALSE))
                {
                    _readFinished = true;
                    if (bytesRead > 0) 
                    {
                        char buf[16];
                        std::string content;

                        for (int i = 0; i < bytesRead; i++) {
                            _inputQueue.push_back(_inputBuffer[i]);
                            sprintf(buf, "%02x ", _inputBuffer[i]);
                            content.push_back(buf[0]);
                            content.push_back(buf[1]);
                            content.push_back(buf[2]);
                        }
                        _totalRead += bytesRead;
                        pLogger->LogInfo("WinRS232::readWriteRS232 read com bytes indirectly: " + std::to_string(bytesRead) + ", totally read: " + std::to_string(_totalRead));
                        pLogger->LogInfo("WinRS232::readWriteRS232 read content 0x: " + content);
                    }
                }
                else
                {
                    auto err = GetLastError();
                    pLogger->LogError("WinRS232::readWriteRS232 failure in overlap com reading: " + std::to_string(err));
                    _state = DeviceState::DeviceError;
                }
                ResetEvent(_readOverlap.hEvent);
            }
            break;
            case WAIT_OBJECT_0 + 1:
            {
                DWORD bytesWritten;
                if (GetOverlappedResult(_handle, &_writeOverlap, &bytesWritten, FALSE))
                {
                    _writeFinished = true;
                    if (bytesWritten > 0)
                    {
                        Poco::ScopedLock<Poco::Mutex> lock(_mutex);

                        char buf[16];
                        std::string content;

                        for (int i = 0; i < bytesWritten; i++) {
                            sprintf(buf, "%02x ", _outputQueue[i]);
                            content.push_back(buf[0]);
                            content.push_back(buf[1]);
                            content.push_back(buf[2]);
                        }

                        _outputQueue.erase(_outputQueue.begin(), _outputQueue.begin() + bytesWritten);
                        _totalWrite += bytesWritten;
                        pLogger->LogInfo("WinRS232::readWriteRS232 wrote com bytes indirectly: " + std::to_string(bytesWritten) + ", totally write: " + std::to_string(_totalWrite));
                        pLogger->LogInfo("WinRS232::readWriteRS232 wrote content 0x: " + content);
                    }
                }
                else
                {
                    auto err = GetLastError();
                    pLogger->LogError("WinRS232::readWriteRS232 failure in overlap com writing: " + std::to_string(err));
                    _state = DeviceState::DeviceError;
                }
                ResetEvent(_writeOverlap.hEvent);
            }
            break;
            case WAIT_TIMEOUT:
            {
                ;//do nothing
            }
            break;
            default:
            {
                pLogger->LogError("WinRS232::readWriteRS232 failure in waiting: " + std::to_string(rc));
                _state = DeviceState::DeviceError;
            }
            break;
        }
    }
}

void WinRS232::runTask()
{
    pLogger->LogInfo("WinRS232::runTask runs for: " + _name);

    for (;;)
    {
        if (isCancelled()) {
            break;
        }
        else if (_bExit) {
            break;
        }

        switch (_state)
        {
        case DeviceState::DeviceConnected:
        {
            sleep(1000);
            openDevice();
        }
        break;

        case DeviceState::DeviceNormal:
        {
            readWriteRS232();
        }
        break;

        case DeviceState::DeviceError:
        {
            _bExit = true;
        }
        break;

        case DeviceState::DeviceNotConnected:
        {
            Poco::File file(_name);

            if (file.exists()) {
                _state = DeviceState::DeviceConnected;
            }
            else {
                pLogger->LogError("WinRS232::runTask device not connect: " + _name);
                sleep(5000); //continue to check device existence 1 second later.
            }
        }
        break;

        default:
        {
            pLogger->LogError("WinRS232::runTask wrong device state in " + _name + " : " + std::to_string((int)_state));
            _bExit = true;
        }
        break;
        }

        processReceivedData();
    }

    pLogger->LogInfo("WinRS232::runTask for " + _name + " existed");
}

void WinRS232::Send(const unsigned char * pData, const unsigned int amount)
{
    Poco::ScopedLock<Poco::Mutex> lock(_mutex);

    if (amount == 0) {
        return;
    }
    if (pData == nullptr) {
        return;
    }
    if (_state != DeviceState::DeviceNormal) {
        return;
    }

    for (unsigned int i = 0; i < amount; i++) {
        _outputQueue.push_back(pData[i]);
    }
}

void WinRS232::processReceivedData()
{
    unsigned int amount;

    if (_inputQueue.empty()) {
        return;
    }
    if (_pPeer == nullptr) {
        return;
    }

    for (amount = 0; amount < MAX_BUFFER_SIZE;) {
        _inputBuffer[amount] = _inputQueue[0];
        amount++;
        _inputQueue.pop_front();
        if (_inputQueue.empty()) {
            break;
        }
    }

    pLogger->LogInfo("WinRS232::processReceivedData send to peer byte amount: " + std::to_string(amount));
    _pPeer->Send(_inputBuffer, amount);
}

void WinRS232::Connect(IDataExchange * pInterface)
{
    _pPeer = pInterface;
}

#endif

