#pragma once
#include "../module/WcpAbstractModule.hpp"
#include <Windows.h>

class WcpModuleHandler
{

public:

    WcpModuleHandler(HINSTANCE h_instance, std::string dll_name, WcpAbstractModule* module);

    HINSTANCE           hInstance() const;
    std::string         dllName() const;
    WcpAbstractModule*  module();

private:

    HINSTANCE           _h_instance;
    std::string         _dll_name;
    WcpAbstractModule*  _module;

};

