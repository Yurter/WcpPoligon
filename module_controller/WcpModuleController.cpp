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


        cv::imwrite("propagateImage.png", WcpModuleUtils::jsonToImage(input_data["image"]));
//        std::cout << input_data["image"];

        for (auto&& module : _module_list) {
            auto ret = module->process(input_data.dump().c_str());
            auto module_answer = nlohmann::json::parse(ret);
            std::cout << "[Debug] module_answer = " << module_answer << std::endl;
    //        output_json.push_back({ module->name(), module_answer });
        }

        return output_json;

    } catch (std::exception e) {
        std::cout << "std::exception: " << e.what() << std::endl;
//        std::string tmp = input_data.dump();
//        std::cout << "input_data: " << input_data << std::endl;
//        int i;
//        std::cin >> i;
    }

    return nlohmann::json();
}
