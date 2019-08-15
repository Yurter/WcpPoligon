#include "WcpModuleConnection.hpp"
#include "../module_base/WcpAbstractModule.hpp"
#include "../module_controller/WcpModuleController.hpp"

void WcpModuleConnection::sendMessage(nlohmann::json message)
{
    switch (ReceiverType(message["receiver_type"])) {
        case ReceiverType::Module: {
            auto module = reinterpret_cast<WcpAbstractModule*>(uint64_t(message["receiver"]));
            module->process(message.dump().c_str());
            return;
        }
        case ReceiverType::Controller: {
            auto controller = reinterpret_cast<WcpModuleController*>(uint64_t(message["receiver"]));
            controller->process(message.dump().c_str());
            return;
        }
    }
}
