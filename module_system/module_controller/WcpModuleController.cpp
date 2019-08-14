#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
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
    _module_list.push_back(module);
    setCallbackFunc(module);
}

void WcpModuleController::processImage(cv::Mat* cvimage)
{
    /* Добавляем каринку в список */
    cv::Mat* img = addImage(nullptr, cvimage->clone());

    nlohmann::json source_image;
    source_image["name"] = "source_image";
    source_image["image"] = reinterpret_cast<uint64_t>(img);
    source_image["object_uid"] = uint64_t(0);
    source_image["parent_uid"] = uint64_t(0);

    std::lock_guard<std::mutex> lock(_heap_mutex);
    _heap.push_back(source_image);
}

void WcpModuleController::addObject(nlohmann::json object)
{
    if (WcpModuleUtils::ckeckJsonField(object, "rect", JsDataType::object) == true) {
        replaceRectWithImage(object);
    }
    std::lock_guard lock(_heap_mutex);
    _heap.push_back(object);
}

void WcpModuleController::createCallbackFunc()
{
    _callback_func = [](uint64_t ctrl_ptr, uint64_t module_ptr, const char* data) {
        /* Декодировка параметров */
        auto this_controller = reinterpret_cast<WcpModuleController*>(ctrl_ptr);
        auto module = reinterpret_cast<WcpAbstractModule*>(module_ptr);
        auto request = nlohmann::json::parse(data);

        /* debug */ std::cout << "callback_func: " << request << std::endl;

        if (WcpModuleUtils::ckeckJsonField(request, "action", JsDataType::string) == false) {
            throw std::exception(R"(invalid or missing "object" field in request)");
        }

        /* Обработка запрашиваемого действия */
        if (request["action"] == "register") {
            if (WcpModuleUtils::ckeckJsonField(request, "object_list", JsDataType::array) == false) {
                throw std::exception(R"(invalid or missing "object_list" field in request)");
            }

            nlohmann::json response;
            for (std::string class_name : request["object_list"]) {
                nlohmann::json jsclass_uid;
                jsclass_uid[class_name] = uint64_t(2);
                response["objects_uid"].push_back(jsclass_uid);
            }

            this_controller->sendCommand(module, response);
            return;
        }

        if (request["action"] == "commit") {
            if (WcpModuleUtils::ckeckJsonField(request, "object", JsDataType::array) == false) {
                throw std::exception(R"(invalid or missing "object_array" field in request)");
            }
            auto object = request["object"];
            this_controller->addObject(object);
            return;
        }
//        if (request["action"] == "commit") {
//            if (WcpModuleUtils::ckeckJsonField(request, "object", JsDataType::object) == false) {
//                throw std::exception(R"(invalid or missing "object" field in request)");
//            }
//            auto object = request["object"];
//            this_controller->addObject(object);
//            return;
//        }
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
            for (auto&& object : heap_dump) {
                propagateObject(object);
            }
        }
    });
    _thread.detach();
}

void WcpModuleController::stopProcessing()
{
    removeCallbackFunc();
    _running = false;
}

void WcpModuleController::setCallbackFunc(WcpAbstractModule* module)
{
    nlohmann::json js_set_callback;
    js_set_callback["action"] = "set_callback";
    js_set_callback["ctrl_ptr"] = reinterpret_cast<uint64_t>(this);
    js_set_callback["callback_func"] = reinterpret_cast<uint64_t>(&_callback_func);

    auto module_answer = nlohmann::json::parse(module->process(js_set_callback.dump().c_str()));

    if (module_answer["status"] == "failed") {
        std::string err_msg = "Failed set callback_func to module: " + std::string(module->header()->name());
        throw std::exception(err_msg.c_str());
    }
}

void WcpModuleController::removeCallbackFunc()
{
    nlohmann::json js_set_callback;
    js_set_callback["action"] = "remove_callback";
    js_set_callback["ctrl_ptr"] = reinterpret_cast<uint64_t>(this);

    for (auto&& module : _module_list) {
        auto module_answer = nlohmann::json::parse(module->process(js_set_callback.dump().c_str()));
        if (module_answer["status"] == "failed") {
            std::string err_msg = "Failed set callback_func to module: " + std::string(module->header()->name());
            throw std::exception(err_msg.c_str());
        }
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

cv::Mat* WcpModuleController::addImage(cv::Mat* parent_cvimage, cv::Mat cvimage)
{
    std::shared_ptr<cv::Mat*> test;
    _sub_images[parent_cvimage].push_back(cvimage);
    return &_sub_images[parent_cvimage].back();
}

void WcpModuleController::removeImage(cv::Mat* parent_cvimage, cv::Mat& cvimage)
{
    _sub_images[parent_cvimage].remove_if([&](cv::Mat& cvsub_image){
        return &cvsub_image == &cvimage;
    });
    if (_sub_images[parent_cvimage].empty()) {
        _sub_images.erase(parent_cvimage);
    }
}

void WcpModuleController::replaceRectWithImage(nlohmann::json& object)
{
    cv::Mat parent_image = WcpModuleUtils::jsonToImage(object["parent_image"]);
    cv::Rect2f rect = WcpModuleUtils::jsonToRect<cv::Rect2f>(object["rect"]);
    cv::Rect roi = WcpModuleUtils::fixRect(rect, parent_image);

    cv::Mat sub_image = parent_image(roi);

    cv::Mat* img = addImage(
                reinterpret_cast<cv::Mat*>(uint64_t(object["parent_image"]))
            , sub_image);
    object["image"] = uint64_t(img);

    object.erase("rect");
}
