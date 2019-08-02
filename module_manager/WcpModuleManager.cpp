#include "WcpModuleManager.hpp"
#include <iostream>

WcpModuleManager::WcpModuleManager(std::string module_path) :
    _module_path(module_path)
{
    //
}

WcpModuleManager::~WcpModuleManager()
{
    if (!_module_list.empty()) {
        unload();
    }
}

void WcpModuleManager::load()
{
    for (auto&& dll_name : getDllNameList()) {
        /* Попытка загрузить dll */
        std::wstring wstr(dll_name.begin(), dll_name.end());
        HINSTANCE h_module = ::LoadLibrary(wstr.c_str());
        if (h_module == nullptr) {
            std::string err_msg = "Failed to load library: " + dll_name;
            throw std::exception(err_msg.c_str());
        }

        /* Попытка получить указатель на функцию создания модуля */
        CreateModuleFunc create_func = CreateModuleFunc(::GetProcAddress(HMODULE(h_module), "createModule"));
        if (create_func == nullptr) {
            std::string err_msg = "Failed to get a create_func: " + dll_name;
            throw std::exception(err_msg.c_str());
        }

        /* Попытка создать объект модуля */
        auto module = create_func();
        if (module == nullptr) {
            std::string err_msg = "Failed to creat module: " + dll_name;
            throw std::exception(err_msg.c_str());
        }

        /* Добавление модуля и его хендлера в список */
        _module_list.push_back({ h_module, module });

        /* Лог */
        std::cout << "Loaded: " << dll_name << std::endl;
    }
}

void WcpModuleManager::unload()
{
    for (auto&& module : _module_list) {
        std::string module_name = module.second->name();
        ::FreeLibrary(module.first);
        /* Лог */
        std::cout << "Unloaded: " << module.second->name() << std::endl;
    }
}

StringList WcpModuleManager::getDllNameList()
{
    //
}
