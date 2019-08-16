#pragma once
//#include "../module_connection/WcpModuleConnection.hpp"
#include "../WcpHeader.hpp"
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "../module_utils/WcpModuleUtils.hpp"
#include "WcpModuleHeader.hpp"
#include "../module_utils/AsyncQueue.hpp"
#include <thread>
#include <atomic>

/* Callback-функция для выполнения запросов из модуля */
//using CallbackFunc = std::function<void(const char*)>;
using CallbackFunc = void(*)(const char*);
//typedef void (*CallbackFunc)(const char*);
using ObjectQueue = AsyncQueue<nlohmann::json>;

#define throw_exception(msg) throw std::exception(std::string(std::string(_header->name()) + " : " + std::string(msg)).c_str())
#define UNUSED(x) (void)x;

/* Класс описывает виртуальный интерфейс для наследующих его модулей */
class WCP_DLL_EXPORT WcpAbstractModule
{

public:

    bool flag = true;

    WcpAbstractModule(WcpModuleHeader* header) :
        _header(header)
//      , _connection(nullptr)
      , _running(false)
    {
        _running = true;
        _thread = std::thread([this]() {
            while (_running) {
                nlohmann::json object;
                guaranteed_pop(_object_queue, object);
//                object["result"] = "failed";
                flag = true;
                onProcess(object);
                if (flag) {
                    finishProcess(object);
                }
//                if (object["result"] == "failed") {
//                    finishProcess(object);
//                }
            }
        });
        _thread.detach();
    }

    virtual ~WcpAbstractModule() {
        _running = false;
    }

    WcpAbstractModule(WcpAbstractModule&) = delete;
    WcpAbstractModule(WcpAbstractModule&&) = delete;

    WcpModuleHeader* header() { return _header; }

    /* Си интерфейс для метода process(const nlohmann::json input_data, nlohmann::json& output_data) : void */
    void process(const char* message) {
        processMessage(nlohmann::json::parse(message));
    }

//    void setConnection(WcpModuleConnection* connection) {
//        _connection = connection;
//    }

protected:

    void processMessage(const nlohmann::json message) {

        std::cout << "received message: " << message << std::endl;

        /* При предусмотренном типе действия, происходит вызов соответствующего метода */
        if (message["action"] == "process") {

//            nlohmann::json object = message["data"]["object"];
            nlohmann::json object = message["data"];

//            std::cout << "pr_object: " << object << std::endl;


            if (!_object_queue.push(object)) { /* ? */ }

            return;
        }

        if (message["action"] == "set_callback") {
//            std::cout << "sender" << message["sender"] << std::endl;
//            std::cout << "data" << message["data"] << std::endl;
//            std::cout << "callback_ptr" <<  message["data"]["callback_ptr"] << std::endl;
            onSetCallback(message["sender"], message["data"]["callback_ptr"]);
//            std::cout << "callback_ptr seted" << std::endl;
            return;
        }

//        if (message["action"] == "set_connection") {
//            onSetCallback(message["data"]["connection_ptr"]);
//            return;
//        }

        if (message["action"] == "remove_callback") {
            return;
        }

        /* При непредусмотренном типе действия, происходит вызов универсального метода */
//        {
//            if (WcpModuleUtils::ckeckJsonField(input_data, "object_array2d", JsDataType::array) == false) {
//                throw_exception("invalid or missing \"object_array2d\" field in input data");
//            }

//            nlohmann::json input_object_array2d = input_data["object_array2d"];
//            onAction(input_data["action"], input_object_array2d);
//            output_data["status"] = "success";
//            return;
//        }

    }

    virtual void onProcess(const nlohmann::json object) = 0;
    virtual void onSetCallback(uint64_t contoller, uint64_t connection_pointer) {
        std::cout << "onSetCallback: " << contoller << " " << connection_pointer << std::endl;
        _callback_list[std::to_string(contoller).c_str()] = connection_pointer;
//        _connection = reinterpret_cast<WcpModuleConnection*>(uint64_t(connection_pointer));
//        registerController();
//        loadData();
    }
    virtual void onRemoveCallback(uint64_t ctrl_pointer) {
//        saveData();
//        _callback_list.erase(ctrl_pointer);
    }
    virtual void onAction(const std::string action, const nlohmann::json input_object_array2d) {
        UNUSED(action) UNUSED(input_object_array2d)
        throw_exception(action + " action called, but virtual function \"onAction\" not overrided");
    }

    /* Метод возвращает данные, которые были сохранены ранее методом writeData */
    nlohmann::json readData() {
        return _module_data;
    }

    /* Метод заменяет данные, которые были сохранены ранее, новыми, переданными в качестве аргумента        */
    /* Дозапись осуществляется путем вызова метода чтения, ручной модификацией данных, вызова метода записи */
    void writeData(nlohmann::json jsupdated_data) {
        _module_data = jsupdated_data;
        saveData();
    }

    /* Метод вызывается внутри реализации onProcess при каждом успешном получении результатов */
    void stashObject(const nlohmann::json& parent, std::string obj_name, nlohmann::json jsobj_value) {
//        std::cout << __FUNCTION__ << std::endl;
//        std::cout << "_objects_uid: " << _objects_uid << std::endl;
//        std::cout << "_objects_uid: " << uint64_t(_objects_uid[obj_name.c_str()]) << std::endl;


        if (WcpModuleUtils::ckeckJsonField(jsobj_value, "rect", JsDataType::object)) {
            jsobj_value["root_image"] = parent["data"]["root_image"];
            jsobj_value["parent_image"] = parent["data"]["pointer"];
        }



        flag = false;

        auto object = WcpModuleUtils::createObject(
                    obj_name
//                    , uint64_t(_objects_uid[obj_name.c_str()])
                    , 0
                    , uint64_t(parent["uid"])
                    , parent["ctrl_ptr"]
                    , jsobj_value);

//        std::cout << "object: " << object << std::endl;
        commitObject(object);
//         std::cout << "stashObject pre\n";
//         std::cout << "_objects_uid: " << _objects_uid << std::endl;
//         std::cout << "obj_name.c_str(): " << obj_name.c_str() << std::endl;
//        _objects_uid[obj_name.c_str()] = uint64_t(_objects_uid[obj_name.c_str()]) + 1;

        std::cout << "stashObject finised\n";
    }
//    /* Метод вызывается внутри реализации onProcess при каждом успешном получении результатов */
//    void stashObject(const nlohmann::json& parent, std::string obj_name, nlohmann::json jsobj_value) {
//        /* Формирование json-объекта, поля котрого содержат массивы однотипных объектов */
//        jsobj_value["object_uid"] = uint64_t(_objects_uid[obj_name]);
//        jsobj_value["parent_uid"] = uint64_t(parent["object_uid"]);
//        _objects_uid[obj_name] = uint64_t(_objects_uid[obj_name]) + 1;
//        _stashed_objects[obj_name].push_back(jsobj_value);
//        /* Запсись полученного объекта в базу данных */
//        nlohmann::json resulting_object;
//        resulting_object[obj_name] = jsobj_value;
//        commitObject(resulting_object);
//    }

private:

    /* Модуль может использовать постоянное внешнее хранилище данных, содержимое которого
     * хранится в произваольной форме. Запись и чтение происходит со всей информацией разом */
    void loadData() {
//        if (_connection == nullptr) {
//            throw_exception("_connection is null");
//        }
//        nlohmann::json request;
//        request["module"] = _header->workname();
//        request["action"] = "load";
//        _module_data = response;
    }

    void saveData() {
////        if (_callback_func == nullptr) {
////            throw_exception("Callback function is null");
////        }
//        nlohmann::json request;
//        nlohmann::json response;
//        request["module"] = _header->workname();
//        request["action"] = "save";
//        request["data"] = _module_data;
////        _callback_func(request, response);
    }

    void registerModule() {
//        if (_callback_func == nullptr) {
//            throw_exception("Callback function is null");
//        }
//        nlohmann::json request;
//        nlohmann::json response;
//        request["module"] = _header->workname();
//        request["action"] = "register";
//        request["object_list"] = _header->resultObjectList();
////        _callback_func(request, response);

//        if (WcpModuleUtils::ckeckJsonField(response, "objects_uid", JsDataType::array) == false) {
//            throw_exception("invalid or missing \"objects_uid\" field in response");
//        }
//        auto objects_uid_array = response["objects_uid"];
//        _objects_uid = WcpModuleUtils::arrayToObject(objects_uid_array);
    }

    void commitObject(nlohmann::json object) {
//        std::cout << __FUNCTION__ << std::endl;
        auto message = WcpModuleUtils::createMessage(
                    ReceiverType::Controller
                    , uint64_t(this)
                    , uint64_t(object["ctrl_ptr"])
                    , "commit"
                    , object);
        sendMessage(message);
    }

    void sendMessage(nlohmann::json message) {
//        std::cout << __FUNCTION__ << std::endl;
//        std::cout << "_callback_list: " << _callback_list << std::endl;
//        std::cout << "receiver: " << message["receiver"].dump().c_str() << std::endl;
//        std::cout << "qwe: " << uint64_t(CallbackFunc(uint64_t(_callback_list[message["receiver"].dump().c_str()]))) << std::endl;
//        auto callbac_func = CallbackFunc(_callback_list[uint64_t(message["receiver"])]);
        auto callbac_func = CallbackFunc(uint64_t(_callback_list[message["receiver"].dump().c_str()]));
//        std::cout << "callbac_func: " << callbac_func << std::endl;
//        callbac_func("Hello world");
        callbac_func(message.dump().c_str());
    }

    void finishProcess(const nlohmann::json object) {
        auto message = WcpModuleUtils::createMessage(
                    ReceiverType::Controller
                    , uint64_t(this)
                    , uint64_t(object["ctrl_ptr"])
                    , "finished"
                    , object);
        sendMessage(message);
    }

private:

    /* Метаданные */
    WcpModuleHeader*    _header;

    ObjectQueue         _object_queue;

//    WcpModuleConnection* _connection;

    /* Вспомогательные члены класса */
    nlohmann::json      _callback_list;     /* Связь от модуля к ядру                                               */
    nlohmann::json      _objects_uid;       /* UID объекта в контексте моудуля                                      */
    std::string         _json_dump_buffer;  /* Возвращаемый указатель метода process(const char* input_data)        *
                                             * ссылается на содержмиое этой строки                                  */
//    nlohmann::json      _stashed_objects;   /* Буффер результирующих объектов                                       */
    nlohmann::json      _module_data;       /* Копия произвольных данных модуля, хранящихся во внешнем хранилище    */

    std::thread         _thread;
    std::atomic_bool    _running;

};
