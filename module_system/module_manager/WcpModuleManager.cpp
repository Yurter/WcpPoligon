#include "WcpModuleManager.hpp"
#include <iostream>
#include <filesystem>

WcpModuleManager::WcpModuleManager(const char *module_path) :
    _module_path(module_path)
{
    //
}

WcpModuleManager::~WcpModuleManager()
{
    if (!_handler_list.empty()) {
        unload();
    }
}

HeaderList WcpModuleManager::availableModules()
{
    HeaderList header_list;
    for (auto&& dll_handler : _handler_list) {
        header_list.push_back(dll_handler.header());
    }
    return header_list;
}

WcpAbstractModule* WcpModuleManager::createModule(WcpModuleHeader* module_header)
{
    auto dll_handler = std::find_if(_handler_list.begin(), _handler_list.end(), [module_header](WcpModuleHandler handler){
            return handler.header() == module_header;
    });
    if (dll_handler == _handler_list.end()) {
        std::string err_msg = std::string("Failed to find dll handler by header: ") + module_header->name();
        throw std::exception(err_msg.c_str());
    }
    return callDllFunction<CreateModuleFunc,WcpAbstractModule*>(*dll_handler, "createModule");
}

void WcpModuleManager::load()
{
    /* Загружаются все найденые в директории dll */
    for (auto&& dll_name : getFileNameList(_module_path, "dll")) {
        /* Поиск среди ранее загруженных dll по имени файла */
        auto ret = std::find_if(_handler_list.begin(), _handler_list.end(), [dll_name](WcpModuleHandler handler){
                return handler.dllName() == dll_name;
        });

        /* Если dll уже загружена, переход к следующему имени dll */
        if (ret == _handler_list.end()) { continue; }

        /* Попытка загрузить dll */
        std::wstring wstr(dll_name.begin(), dll_name.end());
        HINSTANCE h_instance = ::LoadLibrary(wstr.c_str());
        if (h_instance == nullptr) {
            std::string err_msg = "Failed to load library: " + dll_name;
            throw std::exception(err_msg.c_str());
        }

        /* Добавление хендлера dll в список */
        _handler_list.push_back({ h_instance, dll_name });

        /* Лог */
        std::cout << "Loaded: " << dll_name << std::endl;
    }

    /* Чтение заголовков загруженных модулей */
    for (auto&& dll_handler : _handler_list) {
        readHeader(dll_handler);
    }
}

void WcpModuleManager::unload()
{
    for (auto&& module_handler : _handler_list) {
        ::FreeLibrary(module_handler.hInstance());
        /* Лог */
        std::cout << "Unloaded: " << module_handler.dllName() << std::endl;
    }
    _handler_list.clear();
}

StringList WcpModuleManager::getFileNameList(std::string path, std::string extention) const
{
    StringList dll_list;

    try {
        /* Поиск файлов с указанным расширением в указанной директории */
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string str_path = entry.path().string();
            if (str_path.find("." + extention) != std::string::npos) {
                dll_list.push_back(str_path);
            }
        }
    } catch (std::exception e) { /* Указанной дирректории не существует */
        std::cout << "Exception! " << e.what() << std::endl;
        return StringList();
    }

    return dll_list;
}

void WcpModuleManager::readHeader(WcpModuleHandler& dll_handler)
{
    auto header = callDllFunction<CreateHeaderFunc,WcpModuleHeader*>(dll_handler, "readHeader");
    dll_handler.setHeadet(header);
}
