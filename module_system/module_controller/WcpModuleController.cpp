#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
#include <iostream>

WcpModuleController::WcpModuleController() :
    _running(false)
{
    auto test = &WcpModuleController::callbackFunc;
    std::cout << "test: " << test << std::endl;
    _running = true;
    _thread = std::thread([this]() {
        while (_running) {
            std::lock_guard<std::mutex> lock(_heap_mutex);
            for (auto&& object : _heap) {
                propagateObject(object);
            }
            _heap.clear();
        }
    });
}
//{
//    _running = true;
//    _thread = std::thread([this]() {
//        while (_running) {
//            _data_mutex.lock();
//            auto data_list_copy = _data_list;
//            _data_list.clear();
//            _data_mutex.unlock();
//            for (auto&& row : data_list_copy) {
//                processRecursively(row);
//            }
//        }
//    });
//}

WcpModuleController::~WcpModuleController()
{
    _running = false;
    _thread.join();
}

void WcpModuleController::add(WcpAbstractModule* module)
{
    _module_list.push_back(module);
    setCallbackFunc(module);
}

void WcpModuleController::processImage(cv::Mat cvimage)
{
    /* Добавляем каринку в список */
    _image_list.push_back(cvimage);

    nlohmann::json source_image;
    source_image["name"] = "source_image";
    source_image["image"] = reinterpret_cast<uint64_t>(&_image_list.back());
    source_image["object_uid"] = uint64_t(0);
    source_image["parent_uid"] = uint64_t(0);

    _heap.push_back(source_image);
}

typedef void (WcpModuleController::*testFunc)(nlohmann::json,nlohmann::json&);
void WcpModuleController::setCallbackFunc(WcpAbstractModule* module)
{
    nlohmann::json js_set_callback;
    js_set_callback["action"] = "set_callback";
    js_set_callback["callback_func"] = reinterpret_cast<uint64_t>(&WcpModuleController::callbackFunc);
    auto module_answer = nlohmann::json::parse(module->process(js_set_callback.dump().c_str()));
    if (module_answer["status"] == "failed") {
        std::string err_msg = "Failed to callback_func to module: " + std::string(module->header()->name());
        throw std::exception(err_msg.c_str());
    }
}

void WcpModuleController::propagateObject(nlohmann::json object)
{
    for (auto&& module : _module_list) {
        if (std::string(module->header()->explicitDependence()) == object["name"]) {
            nlohmann::json js_process;
            js_process["action"] = "process";
            js_process["object"] = object;

            auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));
            if (module_answer["status"] == "success") {
                // круто
            } else {
                // очередь модуля переполнена
            }
        }
    }
}

/* Рекурсивная обработка работает в контексте одной строки таблицы */
void WcpModuleController::processRecursively(nlohmann::json object_row)
{
    /* Итерация по результирующим объектам отработавших модулей */
    for (auto&& js_class : object_row) {

        /* Поиск среди неотработавших модулей */
        for (auto&& module : _module_list) {

            /* Если в куче содержится поле со значением - массивом объектов,
             * которое может обработать один из модулей, передаем его ему */
            if (js_class["name"] == std::string(module->header()->explicitDependence())) {

                /* Формирование входных данных */
                nlohmann::json js_process;
                js_process["action"] = "process";
                js_process["object_array1d"] = js_class["object_array1d"]; // Список объектов класса

                /* Блокирующая функция */
                auto module_answer = nlohmann::json::parse(module->process(js_process.dump().c_str()));

                if (module_answer["status"] == "success") {
                    /* Ответ модуля разбивается на объекты и добавляется в кучу */
                    for (auto&& answer_elem : module_answer["object_array2d"]) {
                        auto it = answer_elem.begin();
                        nlohmann::json js_resulting_class = {
                            { "name" , it.key() }
                            , { "object_array1d", it.value() }
                        };
                        { /* Проверка результирующих объектов на наличие части изображения */
                            auto&& result_obj_array = js_resulting_class["object_array1d"];
                            bool rect_exist = WcpModuleUtils::keyExist(result_obj_array[0], "rect");
                            ImageList parent_image_parts;
                            /* Прямоугольник в объекте заменяется на изображение */
                            if (rect_exist) {
                                cv::Mat parent_obj_image = WcpModuleUtils::jsonToImage(js_class["image"]);
                                for (auto&& obj : result_obj_array) {
                                    cv::Rect obj_rect = WcpModuleUtils::jsonToRect<cv::Rect>(obj["rect"]);
                                    obj_rect = WcpModuleUtils::fixRect(obj_rect, parent_obj_image);
                                    parent_image_parts.push_back(parent_obj_image(obj_rect));
                                    obj["image"] = WcpModuleUtils::imageToJson(parent_image_parts.back());
                                    obj.erase("rect");
                                }
                            }
                            std::cout << "Debug, js_resulting_class: " << js_resulting_class << std::endl;
                            /* Попытка обработать полученный ответ другими модулями */
                            processRecursively(nlohmann::json::array({js_resulting_class}));
                        }
                    }
                }
            }
        }
    }
}

void WcpModuleController::callbackFunc(nlohmann::json request, nlohmann::json& response)
{

}
