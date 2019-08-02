#pragma once
#include "WcpModuleHandler.hpp"
#include <Windows.h>

using ModuleList = std::list<WcpModuleHandler>;
using StringList = std::list<std::string>;
using CreateModuleFunc = WcpModuleWrapper*(*)(void);

class WcpModuleManager
{

public:

    WcpModuleManager(std::string module_path);
    ~WcpModuleManager();

    void                load();
    void                unload();

private:

    StringList          getDllNameList();

private:

    std::string         _module_path;
    ModuleList          _module_list;

};

