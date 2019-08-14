#pragma once
#include "../module/WcpAbstractModule.hpp"
#include "../module/WcpModuleHeader.hpp"
#include <Windows.h>

/* Класс-обертка вокруг dll и заголовка модуля */
class WCP_DLL_EXPORT WcpModuleHandler
{

public:

    WcpModuleHandler(HINSTANCE h_instance, std::string dll_name);

    HINSTANCE           hInstance() const;
    std::string         dllName()   const;
    WcpModuleHeader*    header()    const;

    void                setHeadet(WcpModuleHeader* header);

private:

    HINSTANCE           _h_instance;
    std::string         _dll_name;
    WcpModuleHeader*    _header;

};
