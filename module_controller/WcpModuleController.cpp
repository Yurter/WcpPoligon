#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
#include <iostream>

void WcpModuleController::add(WcpAbstractModule* module)
{
    _module_list.push_back(module);
}

bool WcpModuleController::initGraph()
{
    for (auto&& module : _module_list) {
        //
    }
    return true;
}

nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
{
    nlohmann::json input_data;

//    input_data["source_image"] = WcpModuleUtils::imageToJson(image);
    input_data["source_image"] = { "image", WcpModuleUtils::imageToJson(image) };

    nlohmann::json output_json = recursion(input_data);

    return output_json;
}

nlohmann::json WcpModuleController::recursion(nlohmann::json input_data)
{
    nlohmann::json output_json;

    /* Итерация по ключам полей входного джейсона */
    for (auto it = input_data.begin(); it != input_data.end(); ++it) {
        for (auto&& module : _module_list) {
            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
            std::cout << std::endl << "it.key() = " << it.key() << std::endl;
            std::cout << "imodule->explicitDependence() = " << module->explicitDependence() << std::endl;
            if (it.key() == std::string(module->explicitDependence())) {

                /* 1 - Обработка поля модулем */
                nlohmann::json js_process = {
                    { "action", "process" }
                    , *it
                };

                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

                if (module_answer["status"] == "success") {
                    output_json.push_back(module_answer);

                    /* 2 - Обработка ответа модуля подмодулями */
                    auto sub_module_answer = recursion(module_answer);

                    if (sub_module_answer["status"] == "success") {
                        output_json.push_back(sub_module_answer);
                    }

                }
            }
        }
    }

    return output_json;
}
