#pragma once
#include "../module/WcpModuleWrapper.hpp"

class WcpModuleHandler
{

public:

    WcpModuleHandler();

private:

    HINSTANCE           _h_instance;
    std::string         _dll_name;
    WcpModuleWrapper*   _module;

};

