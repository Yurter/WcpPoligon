#pragma once
#include <json.hpp>

class WcpModuleConnection
{

public:

    WcpModuleConnection() = default;
    ~WcpModuleConnection() = default;

    void sendMessage(nlohmann::json message);

};

