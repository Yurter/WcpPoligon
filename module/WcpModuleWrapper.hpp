#pragma once
#include "WcpAbstractModule.hpp"

#define WCP_DLL_EXPORT __declspec(dllexport)

class WcpModuleWrapper
{

public:

    WcpModuleWrapper();
    ~WcpModuleWrapper();

    int                 type()      const;
    const char*         name()      const;
    const char*         version()   const;

    const char*         process(const char* input_data);

private:

    WcpAbstractModule*  _module;

};

extern "C" {
    WCP_DLL_EXPORT WcpModuleWrapper* createModule();
}
