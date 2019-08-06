#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
#include <iostream>

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
    std::cout << __FUNCTION__ << std::endl;
    nlohmann::json output_json;
    //temp solution
    nlohmann::json input_data;

    try {

        input_data["action"] = "detect";
        input_data["image"] = WcpModuleUtils::imageToJson(image);

        for (auto&& module : _module_list) {
            auto ret = module->process(input_data.dump().c_str());
            auto module_answer = nlohmann::json::parse(ret);
            std::cout << "[Debug] " << module->name() << "module_answer = " << module_answer << std::endl;
            output_json.push_back({ module->name(), module_answer });
        }

        return output_json;

    } catch (std::exception e) {
        std::cout << "std::exception: " << e.what() << std::endl;
    }

    return nlohmann::json();
}
