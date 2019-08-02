#include "WcpModuleManager.hpp"



WcpModuleManager::~WcpModuleManager()
{
    if (!_module_list.empty()) {
        unload();
    }
}
