#include "WcpModuleHandler.hpp"

WcpModuleHandler::WcpModuleHandler(HINSTANCE h_instance
                                   , std::string dll_name
                                   , WcpAbstractModule* module) :
    _h_instance(h_instance)
  , _dll_name(dll_name)
  , _module(module)
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

WcpAbstractModule* WcpModuleHandler::module()
{
    return _module;
}
