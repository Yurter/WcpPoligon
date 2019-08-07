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

//    _data_list.push_back(WcpModuleUtils::createJsonObject(
//                             "source_image"
//                             , WcpModuleUtils::imageToJson(image))
//                         );

nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
{
    std::cout << "-----------------------" << __FUNCTION__ << std::endl;
    nlohmann::json source_array;
    source_array["source_image"].push_back(WcpModuleUtils::imageToJson(image));



    _data_list.push_back(source_array);

    recursion();

    nlohmann::json output_json;
    std::swap(output_json, _data_list);

    for (auto&& module : _module_list) {
        module->setUsed(false);
    }

    return output_json;
}

//nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
//{
//    std::cout << "-----------------------" << __FUNCTION__ << std::endl;
//    nlohmann::json source_array;
//    source_array.push_back(WcpModuleUtils::imageToJson(image));
//    _data_list.push_back(WcpModuleUtils::createJsonObject(
//                             "source_image"
//                             , source_array)
//                         );

//    recursion();

//    nlohmann::json output_json;
//    std::swap(output_json, _data_list);

//    for (auto&& module : _module_list) {
//        module->setUsed(false);
//    }

//    return output_json;
//}

void WcpModuleController::recursion()
{
    std::cout << "--------" << __FUNCTION__ << " " << _data_list.size() << std::endl;
    /* Итерация по ключам полей входного джейсона */
    for (auto&& data : _data_list) {
        auto jsfield = data.begin(); /* Элемент массива как json для обращения к ключу и значению */
//        std::cout << "jsfield = " << *jsfield << std::endl;
        try {
            std::cout << "jsfield.key() = " << jsfield.key() << std::endl;
            std::cout << "NORm = " << *jsfield << std::endl;
        } catch (...) {
            std::cout << "ERROR = " << *jsfield << std::endl;
        }

        for (auto&& module : _module_list) {
            if (module->used()) { continue; }

            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
            if (data.begin().key() == std::string(module->explicitDependence())) {
                module->setUsed(true); /* Исключение повторного использования модуля */

                /* 1 - Обработка поля модулем */
                nlohmann::json js_process;
                js_process["action"] = "process";
                js_process["input_array"] = jsfield.value();

                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

                if (module_answer["status"] == "success") {
                    _data_list.push_back(module_answer["result"]);

                    std::cout << "1111111 " << module_answer["result"].is_array() << std::endl;
//                    std::cout << "1111111 " << module_answer["result"] << std::endl;

                    /* 2 - Обработка ответа модуля подмодулями */
                    recursion();
                }
            }
        }
    }
}

//void WcpModuleController::recursion()
//{
//    std::cout << "--------" << __FUNCTION__ << std::endl;
//    /* Итерация по ключам полей входного джейсона */
//    for (auto&& data : _data_list) {
//        auto jsfield = data.begin(); /* Элемент массива как json для обращения к ключу и значению */
//        std::cout << "jsfield.key() = " << jsfield.key() << std::endl;

//        for (auto&& module : _module_list) {
//            if (module->used()) { continue; }

//            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
//            if (data.begin().key() == std::string(module->explicitDependence())) {
//                module->setUsed(true); /* Исключение повторного использования модуля */

//                /* 1 - Обработка поля модулем */
//                nlohmann::json js_process;
//                js_process["action"] = "process";
//                js_process["input_array"] = jsfield.value();

//                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

//                if (module_answer["status"] == "success") {
//                    auto result_value = module_answer["result"];
//                    for (auto&& elem : result_value) {
//                         std::cout << "--------elem.is_array() " << elem.is_array() << std::endl;
//                        _data_list.push_back(elem);
//                    }

//                    /* 2 - Обработка ответа модуля подмодулями */
//                    recursion();
//                }
//            }
//        }
//    }
//}


//void WcpModuleController::recursion()
//{
//    std::cout << "--------" << __FUNCTION__ << std::endl;
//    /* Итерация по ключам полей входного джейсона */
//    for (auto&& data : _data_list) {
//        auto jsfield = data.begin(); /* Элемент массива как json для обращения к ключу и значению */
//        std::cout << "jsfield.key() = " << jsfield.key() << std::endl;

//        for (auto&& module : _module_list) {
//            if (module->used()) { continue; }

//            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
//            if (data.begin().key() == std::string(module->explicitDependence())) {
//                module->setUsed(true); /* Исключение повторного использования модуля */

//                if (data.begin().key() == std::string("face"))
//                    std::cout << "*jsfield = " << *jsfield << std::endl;

//                /* 1 - Обработка поля модулем */
//                nlohmann::json js_process;
//                js_process["action"] = "process";
//                js_process["input_array"] = jsfield.value();

//                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

//                if (module_answer["status"] == "success") {
//                    auto result_value = module_answer["result"];
////                    if (data.begin().key() == std::string("motion"))
////                        std::cout << "result_value = " << result_value << std::endl;
//                    for (auto&& elem : result_value) {
////                        if (data.begin().key() == std::string("motion"))
////                            std::cout << "result_value elem = " << elem << std::endl;
//                        _data_list.push_back(elem);
//                    }

//                    /* 2 - Обработка ответа модуля подмодулями */
//                    recursion();
//                }
//            }
//        }
//    }
//}

//void WcpModuleController::recursion()
//{
//    std::cout << "--------" << __FUNCTION__ << std::endl;
//    /* Итерация по ключам полей входного джейсона */
//    for (auto&& data : _data_list) {
//        auto jsfield = data.begin();
//        std::cout << "jsfield.key() = " << jsfield.key() << std::endl;

//        for (auto&& module : _module_list) {
//            if (module->used()) { continue; }

//            /* Если во входном джейсоне есть поле, которое может обработать один из модулей */
//            if (data.begin().key() == std::string(module->explicitDependence())) {
//                module->setUsed(true);

//                /* 1 - Обработка поля модулем */
//                nlohmann::json js_process;
//                auto js_itvalue = data.begin().value().begin();

//                std::cout << ">>js_itvalue.key() = " << js_itvalue.key() << std::endl;
//                if (module->explicitDependence() == std::string("face")) {
//                    std::cout << "888888-2" << std::endl;
//                    std::cout << "js_itvalue = " << *js_itvalue << std::endl;
//                    std::cout << "js_itvalue.value() = " << js_itvalue.value() << std::endl;
//                }

//                js_process["action"] = "process";
//                js_process[js_itvalue.key()] = js_itvalue.value();

//                if (module->explicitDependence() == std::string("face")) {
//                    std::cout << "888888-3" << std::endl;
//                }

//                if (module->explicitDependence() == std::string("face")) {
//                    std::cout << "js_process = " << js_process << std::endl;
//                }

//                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));
//                std::cout << std::endl << "module_answer[\"status\"] = " << module_answer["status"] << std::endl;

//                if (module_answer["status"] == "success") {
//                    auto result_value = module_answer["result"];
//                    for (auto&& elem : result_value) {
////                        std::cout << std::endl << "elem = " << elem << std::endl;
////                        std::cout << "---11 " << _data_list.is_array() << "\n";
//                        _data_list.push_back(elem);
////                        std::cout << "---22\n";
//                    }

////                    std::cout << "sub recursion()---\n";
//                    /* 2 - Обработка ответа модуля подмодулями */
//                    recursion();
//                }
//            } // if (it.key() ==
//        } // for (auto&& module : _module_list)
//    }
//}

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
