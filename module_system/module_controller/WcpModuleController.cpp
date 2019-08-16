#include "WcpModuleController.hpp"
#include "../module_utils/WcpModuleUtils.hpp"
#include <iostream>

WcpModuleController::WcpModuleController(uint64_t controller_handler, CallbackFunc controller_callback) :
    _controller_handler(controller_handler)
  , _controller_callback(controller_callback)
  , _running(false)
{
    createCallbackFunc();
    startProcessing();
}

WcpModuleController::~WcpModuleController()
{
    stopProcessing();
}

void WcpModuleController::add(WcpAbstractModule* module)
{
//    std::cout << __FUNCTION__ << std::endl;
    _module_list.push_back(module);
    setCallbackFunc(module);
}

void WcpModuleController::remove(WcpAbstractModule* module)
{
    _module_list.remove_if([=](WcpAbstractModule* mod){
        return mod == module;
    });
}

void WcpModuleController::processImage(cv::Mat* cvimage)
{
    std::cout << __FUNCTION__ << std::endl;
    /* Добавляем каринку в список */
    _image_list.push_back(cvimage->clone());
    cv::Mat* img = addImage(uint64_t(&_image_list.back()), _image_list.back());
    uint64_t img_ptr = reinterpret_cast<uint64_t>(img);

    std::cout << ">>> (key)source_img: " << uint64_t(&_image_list.back()) << std::endl;

    std::cout << ">>> img_ptr: " << img_ptr << std::endl;
    cv::imwrite("img_ptr.png", WcpModuleUtils::jsonToImage(img_ptr));

    nlohmann::json source_image = WcpModuleUtils::createObject(
                "source_image"
                , 0
                , 0
                , uint64_t(this)
                , WcpModuleUtils::createImage(img_ptr, uint64_t(&_image_list.back()), 0)
                );

    std::lock_guard<std::mutex> lock(_heap_mutex);
    _heap.push_back(source_image);
}

void WcpModuleController::subscribeToObject(std::string object_name)
{
    _subscribe_object_list.push_back(object_name);
}

//void WcpModuleController::processImage(cv::Mat* cvimage)
//{
//    /* Добавляем каринку в список */
//    _image_list.push_back(cvimage->clone());
//    cv::Mat* img = addImage(uint64_t(&_image_list.back()), _image_list.back());

//    nlohmann::json source_image;
//    source_image["name"] = "source_image";
//    source_image["root_image"] = reinterpret_cast<uint64_t>(img);
//    source_image["image"]      = reinterpret_cast<uint64_t>(img);
//    source_image["object_uid"] = uint64_t(0);
//    source_image["parent_uid"] = uint64_t(0);

//    std::lock_guard<std::mutex> lock(_heap_mutex);
//    _heap.push_back(source_image);
//}

void WcpModuleController::commitObject(nlohmann::json object)
{
    /* Отсечение у объекта служебных полей и сохранение его в БД */
    saveObject(object);
    /* Подготовка оъекта к обработке и помещении его в кучу */
    addObject(object);
    /* Уведовление класс верхнего уровня при необходимости */
    notifyHandler(object);
}

void WcpModuleController::process(nlohmann::json message)
{
//    /* Обработка запрашиваемого действия */
//    if (message["action"] == "register") {
//        //
//        return;
//    }

//    if (message["action"] == "commit") {
//        auto object = message["data"]["object"];
//        commitObject(object);
//        return;
//    }
}

void WcpModuleController::createCallbackFunc()
{
    _callback_func = [](const char* message) {
        auto jsmessage = nlohmann::json::parse(message);
        /* Декодировка параметров */
        auto this_controller = reinterpret_cast<WcpModuleController*>(uint64_t(jsmessage["receiver"]));
        auto module = reinterpret_cast<WcpAbstractModule*>(uint64_t(jsmessage["sender"]));

        /* debug */ std::cout << "callback_func, jsmessage: " << jsmessage << std::endl;

        /* Обработка запрашиваемого действия */
        if (jsmessage["action"] == "register") {
            return;
        }

        if (jsmessage["action"] == "commit") {
            auto object = jsmessage["data"];
            this_controller->commitObject(object);
            return;
        }
    };
}

void WcpModuleController::startProcessing()
{
    _running = true;
    _thread = std::thread([this]() {
        while (_running) {
            nlohmann::json heap_dump;
            {
                _heap_mutex.lock();
                heap_dump = _heap;
                _heap.clear();
                _heap_mutex.unlock();
            }
            printTable();
            for (auto&& object : heap_dump) {
                propagateObject(object);
            }
            sleep_for_ms(500);
        }
    });
    _thread.detach();
}

void WcpModuleController::stopProcessing()
{
//    removeCallbackFunc();
    _running = false;
}

void WcpModuleController::setCallbackFunc(WcpAbstractModule* module)
{
    nlohmann::json jscallback;
    jscallback["callback_ptr"] = uint64_t(_callback_func);
    auto message = WcpModuleUtils::createMessage(
                ReceiverType::Module
                , uint64_t(this)
                , uint64_t(module)
                , "set_callback"
                , jscallback
                );
    module->process(message.dump().c_str());
}

void WcpModuleController::propagateObject(nlohmann::json object)
{
    for (auto&& module : _module_list) {
        std::cout << "-> " << module->header()->explicitDependence() << " " << object["name"] << std::endl;
        if (std::string(module->header()->explicitDependence()) == object["name"]) {
            auto message = WcpModuleUtils::createMessage(
                        ReceiverType::Module
                        , uint64_t(this)
                        , uint64_t(module)
                        , "process"
                        , object
                        );
            module->process(message.dump().c_str());
            return;
        }
    }
    removeParentImage(object);
}

cv::Mat* WcpModuleController::addImage(uint64_t root_cvimage, cv::Mat cvimage)
{
    _sub_images[root_cvimage].second.push_back(cvimage);
    _sub_images[root_cvimage].first++;
    return &_sub_images[root_cvimage].second.back();
}

void WcpModuleController::removeImage(uint64_t root_cvimage)
{
    _sub_images[root_cvimage].first--;
    if (_sub_images[root_cvimage].first == 0) {
        _image_list.remove_if([=](cv::Mat& img){ return uint64_t(&img) == root_cvimage; });
        _sub_images.erase(root_cvimage);
    }
}

void WcpModuleController::replaceRoiWithImage(nlohmann::json& object)
{
    uint64_t root_image = object["data"]["root_image"];
    cv::Mat* parent_image = reinterpret_cast<cv::Mat*>(uint64_t(object["data"]["parent_image"]));
    cv::Rect2f rect = WcpModuleUtils::jsonToRect<cv::Rect2f>(object["data"]["rect"]);
    cv::Rect2f roi = WcpModuleUtils::fixRect<cv::Rect2f>(rect, *parent_image);

    cv::Mat sub_image = cv::Mat(*parent_image)(roi);

    cv::Mat* img = addImage(root_image, sub_image);

    object["data"] = WcpModuleUtils::createImage(uint64_t(img)
                                                 , root_image
                                                 , uint64_t(object["data"]["parent_image"]));

    object.erase("rect");
}

void WcpModuleController::removeParentImage(nlohmann::json object)
{
    if (WcpModuleUtils::ckeckJsonField(object["data"], "root_image", JsDataType::number_unsigned) == false) {
        return;
    }
    removeImage(object["data"]["root_image"]);
}

void WcpModuleController::saveObject(nlohmann::json object)
{
    //убрать служебные поля
    if (WcpModuleUtils::ckeckJsonField(object["data"], "pointer", JsDataType::number_unsigned) == true) {
        object["data"].erase("pointer");
    }
    if (WcpModuleUtils::ckeckJsonField(object["data"], "root_image", JsDataType::number_unsigned) == true) {
        object["data"].erase("root_image");
    }
    if (WcpModuleUtils::ckeckJsonField(object["data"], "parent_image", JsDataType::number_unsigned) == true) {
        object["data"].erase("parent_image");
    }
    std::cout << "\nSave object to DB: " << object << std::endl << std::endl;
//    //
//    nlohmann::json message;
//    message["action"] = "save_object";
//    message["object"] = object;
//    _controller_callback(message.dump().c_str());
    sendCommandToHandler("save_object", object);
}

void WcpModuleController::addObject(nlohmann::json object)
{
    std::cout << __FUNCTION__ << std::endl;
    if (WcpModuleUtils::ckeckJsonField(object["data"], "rect", JsDataType::object)) {
        replaceRoiWithImage(object);
        removeParentImage(object);
    } else {
        removeParentImage(object);
    }
    std::lock_guard lock(_heap_mutex);
    _heap.push_back(object);
    std::cout << "Pushed object: " << object << std::endl;
    std::cout << __FUNCTION__ << " finished" << std::endl;
}

void WcpModuleController::notifyHandler(nlohmann::json object)
{
    auto ret = std::find(_subscribe_object_list.begin()
                         , _subscribe_object_list.end()
                         , object["name"]);
//    if (ret != _subscribe_object_list.end()) {
//        nlohmann::json message;
//        message["action"] = "notify";
//        message["object"] = object;
//        _controller_callback(message.dump().c_str());
//    }
    if (ret != _subscribe_object_list.end()) {
        sendCommandToHandler("notify", object);
    }
}

void WcpModuleController::sendCommandToHandler(std::string action, nlohmann::json data)
{
    nlohmann::json message;
    message["action"] = action;
    message["data"] = data;
    message["pointer"] = _controller_handler;
    _controller_callback(message.dump().c_str());
}

void WcpModuleController::printTable()
{
    std::cout << "--------------------------------------" << std::endl;;
    for (auto&& row : _sub_images) {
        std::cout << "key: " << row.first << " ";
        std::cout << "n: " << row.second.first << " ";
        for (auto&& img : row.second.second) {
            std::cout << uint64_t(&img) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------------------------" << std::endl;
}

