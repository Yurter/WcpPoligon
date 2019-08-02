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
        HINSTANCE h_instance = ::LoadLibrary(wstr.c_str());
        if (h_instance == nullptr) {
            std::string err_msg = "Failed to load library: " + dll_name;
            throw std::exception(err_msg.c_str());
        }

        /* Попытка получить указатель на функцию создания модуля */
        CreateModuleFunc create_func = CreateModuleFunc(::GetProcAddress(HMODULE(h_instance), "createModule"));
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
        _module_list.push_back({ h_instance, dll_name, module });

        /* Лог */
        std::cout << "Loaded: " << dll_name << std::endl;
    }
}

void WcpModuleManager::unload()
{
    for (auto&& module_handler : _module_list) {
        ::FreeLibrary(module_handler.hInstance());
        /* Лог */
        std::cout << "Unloaded: " << module_handler.dllName() << std::endl;
    }
    _module_list.clear();
}

StringList WcpModuleManager::getDllNameList()
{
    return StringList();
}
