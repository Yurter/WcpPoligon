#include "WcpModuleHandler.hpp"

WcpModuleHandler::WcpModuleHandler(HINSTANCE h_instance
                                   , std::string dll_name) :
    _h_instance(h_instance)
  , _dll_name(dll_name)
  , _header(nullptr)
{
    //
}

HINSTANCE WcpModuleHandler::hInstance() const
{
    return _h_instance;
}

std::string WcpModuleHandler::dllName() const
{
    return _dll_name;
}

WcpModuleHeader* WcpModuleHandler::header() const
{
    return _header;
}

void WcpModuleHandler::setHeadet(WcpModuleHeader* header)
{
    _header = header;
}
