#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"

void WcpModuleController::add(WcpAbstractModule* module)
{
    _module_list.push_back(module);
}

bool WcpModuleController::initGraph()
{
    return true;
}

nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
{
    nlohmann::json output_json;
    //temp solution
    nlohmann::json input_data;
    input_data["action"] = "detect";
    input_data["image"] = WcpModuleUtils::encodeBase64Image(image);
    for (auto&& module : _module_list) {
        auto ret = module->process(input_data.dump().c_str());
        auto module_answer = nlohmann::json::parse(ret);
        output_json.push_back({ module->name(), module_answer });
    }
    return output_json;
}
