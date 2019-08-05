#pragma once
#include "WcpModuleHandler.hpp"
#include <list>

using HandlerList = std::list<WcpModuleHandler>;
using StringList = std::list<std::string>;
using CreateModuleFunc = WcpAbstractModule*(*)(void);

/* Класс загружает и выгружает модули из dll файлов в указанной директории */
class WcpModuleManager
{

public:

    WcpModuleManager(std::string module_path);
    ~WcpModuleManager();

    StringList          availableModules() const;
    HandlerList*        handlerList();

    void                load();
    void                unload();

private:

    StringList          getFileNameList(std::string path, std::string extention) const;

private:

    std::string         _module_path;
    HandlerList         _handler_list;

};
