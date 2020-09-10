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
    _comEventHappened = true;
    _comWriteFinished = true;

    memset(&_comOverlap, 0, sizeof(_comOverlap));
    _comOverlap.hEvent = CreateEventA(0, true, 0, 0); //manual reset event
    memset(&_writeOverlap, 0, sizeof(_writeOverlap));
    _writeOverlap.hEvent = CreateEventA(0, true, 0, 0);
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
        pLogger->LogError("WinComDevice::openDevice failed to open: " + _name);
        return;
    }

    if (!SetCommMask(_handle, EV_RXCHAR | EV_TXEMPTY)) {
        pLogger->LogError("WinRS232::openDevice failed to set events to comm file");
        return;
    }

    modeStr = "baud=115200 data=8 parity=n stop=1 dtr=off rts=off";
    memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
    port_settings.DCBlength = sizeof(port_settings);
    if (!BuildCommDCBA(modeStr.c_str(), &port_settings))
    {
        pLogger->LogError("WinComDevice::openDevice unable to set comport dcb settings");
        CloseHandle(_handle);
        return;
    }
    if (!SetCommState(_handle, &port_settings))
    {
        pLogger->LogError("WinComDevice::openDevice unable to set comport cfg settings");
        CloseHandle(_handle);
        return;
    }

    Cptimeouts.ReadIntervalTimeout = MAXWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier = 0;
    Cptimeouts.ReadTotalTimeoutConstant = 0; 
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant = 0; 
    if (!SetCommTimeouts(_handle, &Cptimeouts))
    {
        pLogger->LogError("WinComDevice::openDevice unable to set comport time-out settings");
        CloseHandle(_handle);
        return;
    }

    pLogger->LogInfo("WinComDevice::openDevice file is opened: " + _name);
    _comEventHappened = true;
    ResetEvent(_comOverlap.hEvent);
    _state = DeviceState::DeviceNormal;
}

void WinRS232::readWriteRS232()
{
    DWORD dwEventMask = 0;

    if (_comEventHappened) {
        auto rc = WaitCommEvent(_handle, &dwEventMask, &_comOverlap);
        if (!rc) {
            if (GetLastError() != ERROR_IO_PENDING) {
                _state = DeviceState::DeviceError;
                pLogger->LogError("WinRS232::readWriteRS232 failed in waiting for comm event");
                return;
            }
        }
    }

    auto rc = WaitForSingleObject(_comOverlap.hEvent, READING_TIMEOUT);
    switch (rc)
    {
        case WAIT_OBJECT_0:
        {
            ResetEvent(_comOverlap.hEvent);
            _comEventHappened = true;

            if (dwEventMask & EV_RXCHAR) 
            {
                BOOL abRet = false;
                DWORD dwBytesRead = 0;
                OVERLAPPED ovRead;
                memset(&ovRead, 0, sizeof(ovRead));
                ovRead.hEvent = CreateEventA(0, true, 0, 0);
                do
                {
                    ResetEvent(ovRead.hEvent);
                    abRet = ReadFile(_handle, _inputBuffer, MAX_BUFFER_SIZE, &dwBytesRead, &ovRead);
                    if (!abRet) {
                        break;
                    }
                    if (dwBytesRead > 0) {
                        for (int i = 0; i < dwBytesRead; i++) {
                            _inputQueue.push_back(_inputBuffer[i]);
                        }
                    }
                } while (0);
                CloseHandle(ovRead.hEvent);
            }

            if (dwEventMask & EV_TXEMPTY) {
                _comWriteFinished = true;
            }
            if (_comWriteFinished) 
            {
                Poco::ScopedLock<Poco::Mutex> lock(_mutex);

                if (!_outputQueue.empty()) 
                {
                    int amount = 0;

                    for (; (amount < _outputQueue.size()) && (amount < MAX_BUFFER_SIZE); amount++) {
                        _outputBuffer[amount] = _outputQueue[0];
                        _outputQueue.pop_front();                           
                    }

                    ResetEvent(_writeOverlap.hEvent);
                    WriteFile(_handle, _outputBuffer, amount, NULL, &_writeOverlap);
                    _comWriteFinished = false;
                }
            }
        }
        break;
        case WAIT_TIMEOUT:
        {
            _comEventHappened = false;
        }
        break;
        default:
        {
            pLogger->LogError("WinRS232::readWriteRS232 failed in waiting event, error code: " + std::to_string(rc));
            _state = DeviceState::DeviceError;
        }
        break;
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

    _pPeer->Send(_inputBuffer, amount);
}

void WinRS232::Connect(IDataExchange * pInterface)
{
    _pPeer = pInterface;
}













#endif

