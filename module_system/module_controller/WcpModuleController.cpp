#include "WcpModuleController.hpp"
#include "../module_utils/WcpModuleUtils.hpp"
#include <iostream>

WcpModuleController::WcpModuleController() :
    _running(false)
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
    std::cout << __FUNCTION__ << std::endl;
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

    nlohmann::json source_image = WcpModuleUtils::createObject(
                "source_image"
                , 0
                , 0
                , uint64_t(this)
                , WcpModuleUtils::createImage(img_ptr, img_ptr, 0)
                );

    std::lock_guard<std::mutex> lock(_heap_mutex);
    _heap.push_back(source_image);
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

static CallbackFunc test_func = [](const char* message) {
    std::cout << "test_func: " << message << std::endl;
};

void WcpModuleController::createCallbackFunc()
{
    _callback_func = [](const char* message) {
        std::cout << "ctrl call back: " << std::endl;
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
            std::cout << "action commit finished\n";
            return;
        }
    };
    std::cout << "#### " << &_callback_func << std::endl;
}

//void WcpModuleController::createCallbackFunc()
//{
//    _callback_func = [](uint64_t ctrl_ptr, uint64_t module_ptr, const char* data) {
//        /* Декодировка параметров */
//        auto this_controller = reinterpret_cast<WcpModuleController*>(ctrl_ptr);
//        auto module = reinterpret_cast<WcpAbstractModule*>(module_ptr);
//        auto request = nlohmann::json::parse(data);

//        /* debug */ std::cout << "callback_func: " << request << std::endl;

//        if (WcpModuleUtils::ckeckJsonField(request, "action", JsDataType::string) == false) {
//            throw std::exception(R"(invalid or missing "object" field in request)");
//        }

//        /* Обработка запрашиваемого действия */
//        if (request["action"] == "register") {
//            if (WcpModuleUtils::ckeckJsonField(request, "object_list", JsDataType::array) == false) {
//                throw std::exception(R"(invalid or missing "object_list" field in request)");
//            }
//            nlohmann::json response;
//            for (std::string class_name : request["object_list"]) {
//                nlohmann::json jsclass_uid;
//                jsclass_uid[class_name] = uint64_t(2);
//                response["objects_uid"].push_back(jsclass_uid);
//            }
//            this_controller->sendCommand(module, response);
//            return;
//        }

//        if (request["action"] == "commit") {
//            if (WcpModuleUtils::ckeckJsonField(request, "object", JsDataType::array) == false) {
//                throw std::exception(R"(invalid or missing "object_array" field in request)");
//            }
//            auto object = request["object"];
//            this_controller->commitObject(object);
//            return;
//        }
//    };
//}

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
            for (auto&& object : heap_dump) {
                propagateObject(object);
            }
            std::cout << "controller_loop\n";
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
    std::cout << "setCallbackFunc: " << message << std::endl;
    module->process(message.dump().c_str());
}


//void WcpModuleController::setCallbackFunc(WcpAbstractModule* module)
//{
//    auto message = WcpModuleUtils::createMessage(
//                ReceiverType::Module
//                , uint64_t(this)
//                , uint64_t(module)
//                ,
//                )
//    nlohmann::json js_set_callback;
//    js_set_callback["action"] = "set_callback";
//    js_set_callback["ctrl_ptr"] = reinterpret_cast<uint64_t>(this);
//    js_set_callback["callback_func"] = reinterpret_cast<uint64_t>(&_callback_func);

//    module->process(js_set_callback.dump().c_str());
//}

//void WcpModuleController::removeCallbackFunc()
//{
//    nlohmann::json js_set_callback;
//    js_set_callback["action"] = "remove_callback";
//    js_set_callback["ctrl_ptr"] = reinterpret_cast<uint64_t>(this);

//    for (auto&& module : _module_list) {
//        auto module_answer = nlohmann::json::parse(module->process(js_set_callback.dump().c_str()));
//        if (module_answer["status"] == "failed") {
//            std::string err_msg = "Failed set callback_func to module: " + std::string(module->header()->name());
//            throw std::exception(err_msg.c_str());
//        }
//    }
//}

void WcpModuleController::propagateObject(nlohmann::json object)
{
    std::cout << __FUNCTION__ << " object:" << object << std::endl;
    std::cout << "_module_list.size():" << _module_list.size() << std::endl;
    for (auto&& module : _module_list) {
        std::cout << "explicitDependence:" << module->header()->explicitDependence() << std::endl;
        if (std::string(module->header()->explicitDependence()) == object["name"]) {
            auto message = WcpModuleUtils::createMessage(
                        ReceiverType::Module
                        , uint64_t(this)
                        , uint64_t(module)
                        , "process"
                        , object
                        );
            std::cout << "message: " << message << std::endl;
            module->process(message.dump().c_str());
//            _connection->sendMessage(message);
        }
    }
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
        _sub_images.erase(root_cvimage);
    }
}

void WcpModuleController::replaceRoiWithImage(nlohmann::json& object)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "object: " << object << std::endl;
    uint64_t root_image = object["data"]["root_image"];
    std::cout << "root_image: " << root_image << std::endl;
    cv::Mat* parent_image = reinterpret_cast<cv::Mat*>(uint64_t(object["data"]["parent_image"]));
    std::cout << "parent_image: " << parent_image << std::endl;
    cv::Rect2f rect = WcpModuleUtils::jsonToRect<cv::Rect2f>(object["data"]["rect"]);
    std::cout << "rect: " << rect << std::endl;
    cv::Rect2f roi = WcpModuleUtils::fixRect<cv::Rect2f>(rect, *parent_image);
    std::cout << "roi: " << roi << std::endl;

    cv::Mat sub_image = cv::Mat(*parent_image)(roi);
//    std::cout << "sub_image: " << sub_image << std::endl;

    cv::Mat* img = addImage(root_image, sub_image);
//    object["image"] = uint64_t(img);

    object["data"] = WcpModuleUtils::createImage(uint64_t(img)
                                                 , root_image
                                                 , uint64_t(object["data"]["parent_image"]));

    object.erase("rect");
    std::cout << "replaced object: " << object << std::endl;
}

void WcpModuleController::removeParentImage(nlohmann::json object)
{
    if (WcpModuleUtils::ckeckJsonField(object, "root_image", JsDataType::array) == false) {
        return;
    }
    removeImage(object["root_image"]);
}

void WcpModuleController::saveObject(nlohmann::json object)
{
    //убрать служебные поля
    std::cout << "Save object to DB: " << object;
}

void WcpModuleController::addObject(nlohmann::json object)
{
    std::cout << __FUNCTION__ << std::endl;
    std::cout << "object: " << object << std::endl;
    if (WcpModuleUtils::ckeckJsonField(object["data"], "rect", JsDataType::object)) {
        replaceRoiWithImage(object);
    } else {
        removeParentImage(object);
    }
    std::lock_guard lock(_heap_mutex);
    _heap.push_back(object);

    std::cout << "object pushed " << std::endl;
}

