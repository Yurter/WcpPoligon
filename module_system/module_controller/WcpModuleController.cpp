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

    createSourceObject();
    processRecursively();
    resetModules();

    nlohmann::json output_json;
    std::swap(output_json, _processing_data_list);

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

void WcpModuleController::processRecursively()
{
    /* Итерация по результирующим объектам отработавших модулей */
    for (auto&& js_class : _processing_data_list) {

        /* Поиск среди неотработавших модулей */
        for (auto&& module : _module_list) {
            if (module->used()) { continue; }

            /* Если в куче содержится поле со значением - массивом объектов,
             * которое может обработать один из модулей, передаем его ему */
            if (js_class["name"] == std::string(module->explicitDependence())) {
                module->setUsed(true); /* Исключение повторного использования модуля */

                /* Формирование входных данных */
                nlohmann::json js_process;
                js_process["action"] = "process";
                js_process["object_array1d"] = js_class["object_array1d"]; // Список объектов класса

                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

                if (module_answer["status"] == "success") {
                    /* Ответ модуля разбивается на объекты и добавляется в кучу */
                    for (auto&& answer_elem : module_answer["object_array2d"]) {
                        auto it = answer_elem.begin();
                        nlohmann::json js_resulting_class = {
                            { "name" , it.key() }
                            , { "object_array1d", it.value() }
                        };
                        _processing_data_list.push_back(js_resulting_class);
                    }

                    /* Попытка обработать полученный ответ другими модулями */
                    processRecursively();
                }
            }
        }
    }
}

void WcpModuleController::createSourceObject()
{
    nlohmann::json source_object;
    source_object["name"] = "source_image";
    auto jsimage = WcpModuleUtils::imageToJson(_processing_image);
    jsimage["object_uid"] = uint64_t(0);
    jsimage["parent_uid"] = uint64_t(0);
    source_object["object_array1d"].push_back(jsimage);
    _processing_data_list.push_back(source_object);
}

void WcpModuleController::resetModules()
{
    for (auto&& module : _module_list) {
        module->setUsed(false);
    }
}
