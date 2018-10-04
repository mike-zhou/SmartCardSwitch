#pragma once

class IDevice
{
public:
    virtual void SendCommand(const std::string& deviceName, const std::string& command) = 0;
};

