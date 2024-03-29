#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "module_system/module_manager/WcpModuleManager.hpp"
#include "module_system/module_controller/WcpModuleController.hpp"

#define WEB_CAMERA 0

static uint64_t id = 1;

//static CallbackFunc callback_func = [](nlohmann::json request, nlohmann::json& response) {
//    std::cout << "callback_func: " << request << std::endl;

//    if (request["action"] == "register") {
//        nlohmann::json objects_uid;

//        for (std::string class_name : request["object_list"]) {
//            nlohmann::json jsclass_uid;
//            jsclass_uid[class_name] = uint64_t((id++ + 3) * 2);
//            objects_uid.push_back(jsclass_uid);
//        }

//        response["objects_uid"] = objects_uid;
//    }
//};

static CallbackFunc callback_func = [](const char* message){
    auto jsmessage = nlohmann::json::parse(message);
    auto controller = reinterpret_cast<WcpModuleController*>(uint64_t(jsmessage["controller"]));
//    auto user_data = reinterpret_cast<UserData*>(uint64_t(jsmessage["user_data"]));

    if (jsmessage["action"] == "save_object") {
        auto object = jsmessage["data"];
        // user_data->onSaveObject(object);
        return;
    }

    if (jsmessage["action"] == "notify") {
        auto object = jsmessage["data"];
        // user_data->onNotyfy(controller, object);
        return;
    }

    if (jsmessage["action"] == "object_uid") {
        auto object_name = jsmessage["data"];
        // user_data->onObjectUidRequest(controller, object_name);
        return;
    }

    if (jsmessage["action"] == "load_data") {
        // user_data->onLoadData();
        return;
    }

    if (jsmessage["action"] == "save_data") {
        // user_data->onSaveData();
        return;
    }
};



int main()
{

    /* Открытие веб-камеры на чтение */
    cv::VideoCapture source;
    source.open(WEB_CAMERA);
//    source.set(cv::CAP_PROP_FRAME_WIDTH,  6);
//    source.set(cv::CAP_PROP_FRAME_HEIGHT, 6);
    cv::Mat source_image;

    /* Инициализация менеджера модулей */
    WcpModuleManager module_maneger(R"(D:\dev\webcamcloud_server\WcpPoligon\plugins)");
    module_maneger.load();
    for (auto&& module : module_maneger.availableModules()) {
        std::cout << "available module: " << module->name() << std::endl;
    }
    std::cout << std::endl;
    try {
        module_maneger.load();
    } catch (std::exception e) { std::cout << e.what() << std::endl; return -1; }

    WcpModuleController module_controller(0,callback_func);
    for (auto&& module_header : module_maneger.availableModules()) {
        module_controller.add(module_maneger.createModule(module_header));
    }

    /* Устанвока сallback-функции */
//    module_controller.setCallbackFunc(callback_func);

    try {
        { /* Основной цикл */
            while (cv::waitKey(5) != 'q') {
                source >> source_image;

//                resize(source_image, source_image, cv::Size(1,1), 0, 0, cv::INTER_CUBIC); // Для легкого чтения лога json с картинкой

                static int frame_counter = 0; /* Отладочный пропуск кадров для легкого чтения лога */
                frame_counter++;
                if (frame_counter % 40 == 0) {
                    module_controller.processImage(&source_image);

//                    std::cout << "result: " << result << std::endl;
//                    int ii = 0;
//                    for (auto&& elem : result) {
////                        std::cout << "| " << ii++ << " | " << elem.begin().key() << " : " << elem.begin().value().size() << std::endl;
//                        std::cout << "| " << ii++ << " | " << elem["name"] << " : " << elem["object_array1d"].size() << std::endl;
//                    }
                    std::cout << "===========================================" << std::endl;
                }

                /* Увеличение картинки в два раза */
                resize(source_image, source_image, cv::Size(0,0), 2, 2, cv::INTER_CUBIC);
                cv::imshow("Source", source_image);
            }
        }
    } catch (std::exception e) {
        std::cout << "main: std::exception: " << e.what() << std::endl;
        return -1;
    }

    module_maneger.unload();

    return 0;

}
