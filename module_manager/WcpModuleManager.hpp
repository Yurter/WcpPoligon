#pragma once
#include "WcpModuleHandler.hpp"
#include <list>

using ModuleList = std::list<WcpModuleHandler>;
using StringList = std::list<std::string>;
using CreateModuleFunc = WcpAbstractModule*(*)(void);

class WcpModuleManager
{

public:

    WcpModuleManager(std::string module_path);
    ~WcpModuleManager();

    StringList          availableModules() const;

    void                load();
    void                unload();

private:

    StringList          getFileNameList(std::string path, std::string extention) const;

private:

    std::string         _module_path;
    ModuleList          _module_list;

};

