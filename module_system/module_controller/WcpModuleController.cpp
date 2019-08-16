#include "WcpModuleController.hpp"
#include "../module_utils/WcpModuleUtils.hpp"
#include <iostream>
#include <Remotery.h>

WcpModuleController::WcpModuleController(uint64_t user_data, CallbackFunc controller_callback) :
    _user_data(user_data)
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

    {
        rmt_ScopedCPUSample(Uclone, 0);
        _image_list.push_back(cvimage->clone());
    }


    rmt_BeginCPUSample(UaddImage, 0);

    cv::Mat* img = addImage(uint64_t(&_image_list.back()), _image_list.back());
    uint64_t img_ptr = reinterpret_cast<uint64_t>(img);

    rmt_EndCPUSample();

    std::cout << ">>> (key)source_img: " << uint64_t(&_image_list.back()) << std::endl;

    std::cout << ">>> img_ptr: " << img_ptr << std::endl;
//    cv::imwrite("img_ptr.png", WcpModuleUtils::jsonToImage(img_ptr));

    rmt_BeginCPUSample(UcreateObject, 0);
    nlohmann::json source_image = WcpModuleUtils::createObject(
                "source_image"
                , 0
                , 0
                , uint64_t(this)
                , WcpModuleUtils::createImage(img_ptr, uint64_t(&_image_list.back()), 0)
                );

    rmt_EndCPUSample();

    rmt_BeginCPUSample(Ulock, 0);
    std::lock_guard<std::mutex> lock(_heap_mutex);
    _heap.push_back(source_image);

    rmt_EndCPUSample();
}

void WcpModuleController::subscribeToObject(const char* object_name)
{
    _subscribe_object_list.push_back(object_name);
}

void WcpModuleController::hookObject(const char* object_name)
{
    _hook_object_list.push_back(object_name);
}

void WcpModuleController::loadAll(WcpModuleManager* manager)
{
    for (auto&& module_header : manager->availableModules()) {
        add(manager->createModule(module_header));
    }
}

CallbackFunc WcpModuleController::link()
{
    return _callback_func;
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
    /* Уведовление класс верхнего уровня при необходимости */
    notifyHandler(object);
    /* ? */
    if (hook(object)) {
        return;
    }
    /* Отсечение у объекта служебных полей и сохранение его в БД */
    saveObject(object);
    /* Подготовка оъекта к обработке и помещении его в кучу */
    addObject(object);
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
        std::cout << "$$$$$$$$$$$$$$$$$$ " << message << std::endl;
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

        if (jsmessage["action"] == "unhook") {
            auto object = jsmessage["data"];
            this_controller->unhook(object);
            return;
        }

        if (jsmessage["action"] == "discard") {
            auto object = jsmessage["data"];
            this_controller->discard(object);
            return;
        }

        if (jsmessage["action"] == "finished") {
            std::cout << "\n" << "---------------------------\n\n";
            auto object = jsmessage["data"];
            std::cout << "finished object: " << object << std::endl;
            this_controller->removeParentImage(object);
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

            rmt_ScopedCPUSample(UpropagateObject, 0);
            for (auto&& object : heap_dump) {
                propagateObject(object);
            }
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

bool WcpModuleController::hook(nlohmann::json object)
{
    auto ret = std::find(_hook_object_list.begin()
                         , _hook_object_list.end()
                         , object["name"]);
    if (ret != _hook_object_list.end()) {
        sendCommandToHandler("hook", object);
        return true;
    }
    return false;
}

void WcpModuleController::unhook(nlohmann::json object)
{
    /* Уведовление класс верхнего уровня при необходимости */
    notifyHandler(object);
    /* Отсечение у объекта служебных полей и сохранение его в БД */
    saveObject(object);
    /* Подготовка оъекта к обработке и помещении его в кучу */
    addObject(object);
}

void WcpModuleController::discard(nlohmann::json object)
{
    std::cout << "]]]]]]]]]]]]]]]]]discarded_object: " << object << std::endl;
    removeParentImage(object);
}

void WcpModuleController::sendCommandToHandler(std::string action, nlohmann::json data)
{
    nlohmann::json message;
    message["action"] = action;
    message["data"] = data;
    message["controller"] = uint64_t(this);
    message["user_data"] = _user_data;
    _controller_callback(message.dump().c_str());
}

void WcpModuleController::printTable()
{
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "table size: " << _sub_images.size() << std::endl;
    std::cout << "images size: " << _image_list.size() << std::endl;
    std::cout << "heap size: " << _heap.size() << std::endl;
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

