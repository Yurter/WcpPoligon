#pragma once
#include "WcpModuleHandler.hpp"
#include "../module/WcpModuleHeader.hpp"
#include <list>

using HandlerList = std::list<WcpModuleHandler>;
using HeaderList = std::list<WcpModuleHeader*>;
using StringList = std::list<std::string>;
using CreateModuleFunc = WcpAbstractModule*(*)(void);
using CreateHeaderFunc = WcpModuleHeader*(*)(void);

/* Класс загружает и выгружает модули из dll файлов в указанной директории */
class WcpModuleManager
{

public:

    WcpModuleManager(std::string module_path);
    ~WcpModuleManager();

    /* Метод возвращает список заголовков модулей, доступных для создания */
    HeaderList          availableModules();
    /* Метод возвращает объект модуля, заголовок которого передан аргументом */
    WcpAbstractModule*  createModule(WcpModuleHeader* module_header);

    void                load();
    void                unload();

private:

    StringList          getFileNameList(std::string path, std::string extention) const;

    void                readHeader(WcpModuleHandler& dll_handler);

    template<typename ModuleFunc, class ReturnValue>
    [[nodiscard]] ReturnValue callDllFunction(WcpModuleHandler& module_handler, std::string func_name) {
        /* Попытка получить указатель на функцию */
        auto module_func = ModuleFunc(::GetProcAddress(HMODULE(module_handler.hInstance()), func_name.c_str()));
        if (module_func == nullptr) {
            std::string err_msg = "Failed to get a create_func: " + module_handler.dllName();
            throw std::exception(err_msg.c_str());
        }
        /* Попытка создать объект */
        ReturnValue return_value = module_func();
        if (return_value == nullptr) {
            std::string err_msg = "Failed to call " + func_name + " in " + module_handler.dllName();
            throw std::exception(err_msg.c_str());
        }
        return return_value;
    }

private:

    std::string         _module_path;   /* Относительный путь к директории, в котрой расположены модули */
    HandlerList         _handler_list;  /* Список хендлеров подгруженных dll                            */

};
