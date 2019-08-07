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

    std::cout << "111---\n";
    _data_list.push_back(WcpModuleUtils::createJsonObject(
                             "source_image"
                             , WcpModuleUtils::imageToJson(image))
                         );
    std::cout << "222---\n";

//    _data_list["source_image"] = WcpModuleUtils::imageToJson(image);


    recursion();

    nlohmann::json output_json;
    std::swap(output_json, _data_list);

    for (auto&& module : _module_list) {
        module->setUsed(false);
    }

    return output_json;
}

void WcpModuleController::recursion()
{
//    std::cout << std::endl << "_data_list = " << _data_list << std::endl;
    /* Итерация по ключам полей входного джейсона */
//    for (auto it = _data_list.begin(); it != _data_list.end(); ++it) {
    for (auto&& data : _data_list) {
        std::cout << "recursion()---\n";

        std::cout << std::endl << "it = " << data.is_object() << " " << data.is_array() << std::endl;

//    for (auto&& it : _data_list) {
        for (auto&& module : _module_list) {

            if (module->used()) { continue; }

//            std::cout << std::endl << "*it = " << *it << std::endl;
            std::cout << std::endl << "data = " << data.begin().key() << std::endl;
            std::cout << "imodule->explicitDependence() = " << module->explicitDependence() << std::endl;
            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
            if (data.begin().key() == std::string(module->explicitDependence())) {

                module->setUsed(true);

                /* 1 - Обработка поля модулем */
                nlohmann::json js_process;
                auto js_itvalue = data.begin().value().begin();

                js_process["action"] = "process";
                js_process[js_itvalue.key()] = js_itvalue.value();

                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));
                std::cout << std::endl << "module_answer[\"status\"] = " << module_answer["status"] << std::endl;

                if (module_answer["status"] == "success") {
                    auto result_value = module_answer["result"];
                    for (auto&& elem : result_value) {
//                        std::cout << std::endl << "elem = " << elem << std::endl;
                        std::cout << "---11 " << _data_list.is_array() << "\n";
                        _data_list.push_back(elem);
                        std::cout << "---22\n";
                    }

                    std::cout << "sub recursion()---\n";
                    /* 2 - Обработка ответа модуля подмодулями */
                    recursion();
                }
            } // if (it.key() ==
        } // for (auto&& module : _module_list)
    }
}

//void WcpModuleController::recursion()
//{
////    std::cout << std::endl << "_data_list = " << _data_list << std::endl;
//    /* Итерация по ключам полей входного джейсона */
//    for (auto it = _data_list.begin(); it != _data_list.end(); ++it) {
////    for (auto&& it : _data_list) {
//        for (auto&& module : _module_list) {

////            std::cout << std::endl << "*it = " << *it << std::endl;
//            std::cout << std::endl << "it.key() = " << it.key() << std::endl;
//            std::cout << "imodule->explicitDependence() = " << module->explicitDependence() << std::endl;
//            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
//            if (it.key() == std::string(module->explicitDependence())) {

//                /* 1 - Обработка поля модулем */
//                nlohmann::json js_process = {
//                    { "action", "process" }
//                    , it.value()
//                };
//                std::cout << std::endl << "js_process = " << js_process << std::endl;

//                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));
//                std::cout << std::endl << "module_answer = " << module_answer << std::endl;

//                if (module_answer["status"] == "success") {
//                    auto result_value = module_answer["result"];
//                    for (auto&& elem : result_value) {
//                        _data_list.push_back(elem);
//                    }

//                    /* 2 - Обработка ответа модуля подмодулями */
//                    recursion();
//                }
//            }
//        }
//    }
//}

//nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
//{
//    nlohmann::json input_data;

////    input_data["source_image"] = WcpModuleUtils::imageToJson(image);
//    input_data["source_image"] = { "image", WcpModuleUtils::imageToJson(image) };

//    nlohmann::json output_json = recursion(input_data);

//    return output_json;
//}

//nlohmann::json WcpModuleController::recursion(nlohmann::json input_data)
//{
//    nlohmann::json output_json;

//    /* Итерация по ключам полей входного джейсона */
//    for (auto it = input_data.begin(); it != input_data.end(); ++it) {
//        for (auto&& module : _module_list) {
//            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
//            std::cout << std::endl << "it.key() = " << it.key() << std::endl;
//            std::cout << "imodule->explicitDependence() = " << module->explicitDependence() << std::endl;
//            if (it.key() == std::string(module->explicitDependence())) {

//                /* 1 - Обработка поля модулем */
//                nlohmann::json js_process = {
//                    { "action", "process" }
//                    , *it
//                };

//                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

//                if (module_answer["status"] == "success") {
//                    output_json.push_back(module_answer);

//                    /* 2 - Обработка ответа модуля подмодулями */
//                    auto sub_module_answer = recursion(module_answer);

//                    if (sub_module_answer["status"] == "success") {
//                        output_json.push_back(sub_module_answer);
//                    }

//                }
//            }
//        }
//    }

//    return output_json;
//}
