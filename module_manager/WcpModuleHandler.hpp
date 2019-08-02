#pragma once
#include "../module/WcpModuleWrapper.hpp"
#include <Windows.h>

class WcpModuleHandler
{

public:

    WcpModuleHandler(HINSTANCE h_instance, std::string dll_name, WcpModuleWrapper* module);

    HINSTANCE           hInstance() const;
    std::string         dllName() const;
    WcpModuleWrapper*   module();

private:

    HINSTANCE           _h_instance;
    std::string         _dll_name;
    WcpModuleWrapper*   _module;

};

