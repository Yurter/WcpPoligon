#pragma once
#include "module/WcpModuleWrapper.hpp"

using ModuleList = std::list<WcpModuleWrapper*>;

class WcpModuleManager
{

public:

    WcpModuleManager(std::string mudule_path);
    ~WcpModuleManager();

    void                load();
    void                unload();

private:

    std::string         _mudule_path;
    ModuleList          _module_list;

};

