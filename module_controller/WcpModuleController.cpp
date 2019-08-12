#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
#include <iostream>

void WcpModuleController::add(WcpAbstractModule* module)
{
    _module_list.push_back(module);
}

nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
{
    _processing_image = image;

    nlohmann::json source_array;
    source_array["source_image"].push_back(WcpModuleUtils::imageToJson(_processing_image));
    _data_list.push_back(source_array);

    recursion();

    nlohmann::json output_json;
    std::swap(output_json, _data_list);

    for (auto&& module : _module_list) {
        module->setUsed(false);
    }

    return output_json;
}

void WcpModuleController::setCallbackFunc(CallbackFunc callback_func)
{
    nlohmann::json js_set_callback;
    js_set_callback["action"] = "set_callback";
    js_set_callback["callback_func"] = reinterpret_cast<int64_t>(callback_func);
    for (auto&& module : _module_list) {
       auto module_answer = nlohmann::json::parse(module->process(js_set_callback.dump().c_str()));
       if (module_answer["status"] == "failed") {
           std::string err_msg = "Failed to callback_func to module: " + std::string(module->name());
           throw std::exception(err_msg.c_str());
       }
    }
}

void WcpModuleController::recursion()
{
    /* Итерация по ключам полей входного джейсона */
    for (auto&& data : _data_list) {
        auto js_data = data.begin(); /* Элемент массива как json для обращения к ключу и значению */

        if (js_data == data.end()) {
            throw std::exception("jsdata == data.end()");
        }

        for (auto&& module : _module_list) { /* Поиск среди неотработавших модулей */
            if (module->used()) { continue; }

            /* Если в куче содержится поле со значением - массивом объектов,
             * которое может обработать один из модулей, передаем его ему */
            if (js_data.key() == std::string(module->explicitDependence())) {
                module->setUsed(true); /* Исключение повторного использования модуля */

                /* Формирование входных данных */
                nlohmann::json js_process;
                js_process["action"] = "process";
                js_process["data_array"] = js_data.value();

                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

                if (module_answer["status"] == "success") {
                    /* Ответ модуля разбивается на объекты и добавляется в кучу */
                    for (auto&& answer_elem : module_answer["data_array"]) {
                        _data_list.push_back(answer_elem);
                    }

                    /* Попытка обработать полученный ответ другими модулями */
                    recursion();
                }
            }
        }
    }
}
