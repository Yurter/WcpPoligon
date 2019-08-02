#include "WcpAbstractModule.hpp"

WcpAbstractModule::WcpAbstractModule(ModuleType type
                                     , std::string name
                                     , std::string version
                                     , ModuleContext context_sensitive
                                     , std::string workdir) :
      _type(type)
    , _name(name)
    , _version(version)
    , _context_sensitive(context_sensitive)
    , _workdir(workdir)
{
    //
}
