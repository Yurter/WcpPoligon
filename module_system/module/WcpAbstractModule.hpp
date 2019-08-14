#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "WcpModuleUtils.hpp"
#include "WcpModuleHeader.hpp"
#include "AsyncQueue.hpp"
#include <thread>

#define WCP_DLL_EXPORT __declspec(dllexport)

/* Callback-функция для выполнения запросов из модуля */
using CallbackFunc = void(*)(uint64_t ctrl_ptr, nlohmann::json request, nlohmann::json& response);

using ObjectQueue = AsyncQueue<nlohmann::json>;

#define throw_exception(msg) throw std::exception(std::string(std::string(_header.name()) + " : " + std::string(msg)).c_str())
#define UNUSED(x) (void)x;

/* Класс описывает виртуальный интерфейс для наследующих его модулей */
class /*WCP_DLL_EXPORT*/ WcpAbstractModule
{

public:

    WcpAbstractModule(WcpModuleHeader header) :
        _header(header)
      , _callback_func(nullptr)
      , _objects_uid(0)
      , _running(false)
    {
        _running = true;
        _thread = std::thread([this]() {
            while (_running) {
                nlohmann::json object;
                guaranteed_pop(_object_queue, object);
                onProcess(object);
            }
        });
    }

    virtual ~WcpAbstractModule() {
        _running = false;
        _thread.join();
    }

    WcpAbstractModule(WcpAbstractModule&) = delete;
    WcpAbstractModule(WcpAbstractModule&&) = delete;

    WcpModuleHeader* header() { return &_header; }

    /* Си интерфейс для метода process(const nlohmann::json input_data, nlohmann::json& output_data) : void */
    [[nodiscard]] const char* process(const char* input_data) {
        nlohmann::json output_data;
        process(nlohmann::json::parse(input_data), output_data);
        _json_dump_buffer = output_data.dump();
        return _json_dump_buffer.c_str();
    }

protected:

    void process(const nlohmann::json input_data, nlohmann::json& output_data) {

        if (WcpModuleUtils::ckeckJsonField(input_data, "action", JsDataType::string) == false) {
            throw_exception("invalid or missing \"action\" field in input data");
        }

        /* При предусмотренном типе действия, происходит вызов соответствующего метода */
        if (input_data["action"] == "process") {
            if (WcpModuleUtils::ckeckJsonField(input_data, "object", JsDataType::array) == false) {
                throw_exception("invalid or missing \"object\" field in input data");
            }

            nlohmann::json object = input_data["object"];
            if (!_object_queue.push(object)) {
                output_data["status"] = "failed";
                output_data["error"] = "the queue is full";
                return;
            }

            output_data["status"] = "success";
            return;
        }

        if (input_data["action"] == "set_callback") {
            if (WcpModuleUtils::ckeckJsonField(input_data, "callback_func", JsDataType::number_unsigned) == false) {
                throw_exception("invalid or missing \"callback_func\" field in input data");
            }
            if (WcpModuleUtils::ckeckJsonField(input_data, "ctrl_ptr", JsDataType::number_unsigned) == false) {
                throw_exception("invalid or missing \"ctrl_ptr\" field in input data");
            }

            onSetCallback(input_data["ctrl_ptr"], input_data["callback_func"]);
            output_data["status"] = "success";
            return;
        }

        /* При непредусмотренном типе действия, происходит вызов универсального метода */
        {
            if (WcpModuleUtils::ckeckJsonField(input_data, "object_array2d", JsDataType::array) == false) {
                throw_exception("invalid or missing \"object_array2d\" field in input data");
            }

            nlohmann::json input_object_array2d = input_data["object_array2d"];
            onAction(input_data["action"], input_object_array2d);
            output_data["status"] = "success";
            return;
        }

    }

    virtual void onProcess(const nlohmann::json object) = 0;
    virtual void onSetCallback(uint64_t ctrl_pointer, uint64_t func_pointer) {
        _callback_list[ctrl_pointer] = func_pointer;
        registerController();
        loadData();
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
        /* Формирование json-объекта, поля котрого содержат массивы однотипных объектов */
        jsobj_value["object_uid"] = uint64_t(_objects_uid[obj_name]);
        jsobj_value["parent_uid"] = uint64_t(parent["object_uid"]);
        _objects_uid[obj_name] = uint64_t(_objects_uid[obj_name]) + 1;
        _stashed_objects[obj_name].push_back(jsobj_value);
        /* Запсись полученного объекта в базу данных */
        nlohmann::json resulting_object;
        resulting_object[obj_name] = jsobj_value;
        sendResultingObject(resulting_object);
    }

private:

    /* Метод вызывается внутри реализации onProcess при завршении обработки входных данных и формировании ответа */
    nlohmann::json dumpObjects() {
        /* Преобразование полей json-объекта в массив */
        auto jsdump = WcpModuleUtils::objectToArray(_stashed_objects);
        _stashed_objects.clear();
        return jsdump;
    }

    /* Модуль может использовать постоянное внешнее хранилище данных, содержимое которого
     * хранится в произваольной форме. Запись и чтение происходит со всей информацией разом */
    void loadData() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _header.workname();
        request["action"] = "load";
        _callback_func(request, response);
        _module_data = response;
    }

    void saveData() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _header.workname();
        request["action"] = "save";
        request["data"] = _module_data;
        _callback_func(request, response);
    }

    void registerController() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _header.workname();
        request["action"] = "register";
        request["object_list"] = _header.resultObjectList();
        _callback_func(request, response);

        if (WcpModuleUtils::ckeckJsonField(response, "objects_uid", JsDataType::array) == false) {
            throw_exception("invalid or missing \"objects_uid\" field in response");
        }
        auto objects_uid_array = response["objects_uid"];
        _objects_uid = WcpModuleUtils::arrayToObject(objects_uid_array);
    }

    void sendResultingObject(nlohmann::json jsobject) {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _header.workname();
        request["action"] = "send";
        request["object"] = jsobject;
        _callback_func(request, response);
    }

private:

    /* Метаданные */
    WcpModuleHeader     _header;

    ObjectQueue         _object_queue;

    /* Вспомогательные члены класса */
    nlohmann::json      _callback_list;
//    CallbackFunc        _callback_func;     /* Связь от модуля к ядру                                               */
    nlohmann::json      _objects_uid;       /* UID объекта в контексте моудуля                                      */
    std::string         _json_dump_buffer;  /* Возвращаемый указатель метода process(const char* input_data)        *
                                             * ссылается на содержмиое этой строки                                  */
    nlohmann::json      _stashed_objects;   /* Буффер результирующих объектов                                       */
    nlohmann::json      _module_data;       /* Копия произвольных данных модуля, хранящихся во внешнем хранилище    */

    std::thread         _thread;
    volatile bool       _running;

};
