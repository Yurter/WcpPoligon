#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "WcpModuleUtils.hpp"

#define WCP_DLL_EXPORT __declspec(dllexport)

/* Тип модуля определяет область в которой определено его поведение */
enum ModuleType {
    MotionDetector,     /* Детекция движения на основе последовательности кадров        */
    ObjectDetector,     /* Детекция и классификация объектов на изображении             */
    Recognition,        /* Распознание объектов на изображении (лица, текст)            */
    Tracking,           /* Слежение за объектом в последовательности кадров на видео    */
    Helper,             /* Модуль модифицирует изображение, не сообщает метаинформации  */
    Logic               /* Сбор и анализ данных, полученных от других модулей           */
};

/* Callback-функция для выполнения запросов из модуля */
using CallbackFunc = void(*)(nlohmann::json request, nlohmann::json& response);

#define throw_exception(msg) throw std::exception(std::string(_name + " : " + std::string(msg)).c_str())
#define UNUSED(x) (void)x;

/* Класс описывает виртуальный интерфейс для наследующих его модулей */
class /*WCP_DLL_EXPORT*/ WcpAbstractModule
{

public:

    WcpAbstractModule(ModuleType type                   /* Реализуемый тип модуля                                       */
                      , std::string name                /* Имя модуля в произвольной форме, отображается пользователю   */
                      , std::string workname            /* Имя модуля в уникальной, лаконичной, неизменной от версии к  *
                                                         * версии форме, используется в качестве:                       *
                                                         * 1. метки при хранении данных модуля во внешнем постоянном    *
                                                         * хранилище;                                                   *
                                                         * 2. имени индивидуальной для кажого модуля директорей, в      *
                                                         * которой находятся dll файл и необходимые для работы файлы    *
                                                         * 3. имени контроллера для сетевого взаимодейтсвия             */
                      , std::string version             /* Версия модуля в формате чисел разделенных точками            */
                      , std::string explicit_dependence /* Имя желаемого объекта, который модуль может обработать       */
                      , std::string implicit_dependence /* Имя допустимого объекта, который модуль может обработать     */
                      ) :
        _type(type)
      , _name(name)
      , _workname(workname)
      , _version(version)
      , _implicit_dependence(implicit_dependence)
      , _explicit_dependence(explicit_dependence)
      , _callback_func(nullptr)
      , _used(false)
    { }

    WcpAbstractModule(WcpAbstractModule&) = delete;
    WcpAbstractModule(WcpAbstractModule&&) = delete;

    virtual ~WcpAbstractModule() = default;

    int                 type()                  const { return _type;                           }
    const char*         name()                  const { return _name.c_str();                   }
    const char*         version()               const { return _version.c_str();                }
    const char*         implicitDependence()    const { return _implicit_dependence.c_str();    }
    const char*         explicitDependence()    const { return _explicit_dependence.c_str();    }
    bool                used()                  const { return _used;                           }

    void                setUsed(bool used) { _used = used; }

    /* Си интерфейс для метода process(const nlohmann::json input_data, nlohmann::json& output_data) : void */
    [[nodiscard]] const char* process(const char* input_data) {
        nlohmann::json output_data;
        process(nlohmann::json::parse(input_data), output_data);
        _json_dump_buffer = output_data.dump();
        return _json_dump_buffer.c_str();
    }

protected:

    /* Требования к формату данных:
     * * На вход:
     * {
     *      "action"     : String,          ex.: "process"
     *      "data_array" : Array            ex. 1: [ face_0, face_1, face_2, ... face_N ]   Распознание лиц
     *                                      ex. 2: [ car_0, car_1, car_2, ... car_N ]       Распознание автомобильных номеров
     * }
     * * На выход:
     * {
     *      "status"     : String,    ex.: "success"/"failed"
     *      "data_array" : Array      ex. 1: [ "cat" : cat_array, "dog" : dog_array, ... "smth" : smth_array ] Мультиобъектный детектор
     *                                ex. 2: [ "face" : face_array ]                                           Монообъектный детектор
     * }
    **/
    void process(const nlohmann::json input_data, nlohmann::json& output_data) {

        /* Проверка наличия полей с определенными ключами */
        if (WcpModuleUtils::keyExist(input_data, "action") == false) {
            throw_exception("missing key \"action\" in input data");
        }

        /* Проверка типа полей */
        if (input_data["action"].is_string() == false) {
            throw_exception("value of \"action\" field is not a string");
        }

        /* При предусмотренном типе действия, происходит вызов соответствующего метода */
        if (input_data["action"] == "process") {
            if (WcpModuleUtils::keyExist(input_data, "data_array") == false) {
                throw_exception("missing key \"data_array\" in input data");
            }
            if (input_data["data_array"].is_array() == false) {
                throw_exception("value of \"data_array\" field is not an array");
            }

            nlohmann::json input_data_array = input_data["data_array"];
            nlohmann::json output_data_array;
            if (onProcess(input_data_array, output_data_array) == false) {
                output_data["status"] = "failed";
                return;
            }
            /* Проверка ответа модуля */
            if (output_data_array.is_array() == false) {
                throw_exception("returned value is not an array");
            }
            output_data["status"] = "success";
            output_data["data_array"] = output_data_array;
            return;
        }

        if (input_data["action"] == "set_callback") {
            if (WcpModuleUtils::keyExist(input_data, "callback_func") == false) {
                throw_exception("missing key \"callback_func\" in input data");
            }
            if (input_data["callback_func"].is_number() == false) {
                throw_exception("value of \"callback_func\" field is not a number");
            }
            if (onSetCallback(input_data["callback_func"]) == false) {
                output_data["status"] = "failed";
                return;
            }
            output_data["status"] = "success";
            return;
        }

        /* При непредусмотренном типе действия, происходит вызов универсального метода */
        {
            if (WcpModuleUtils::keyExist(input_data, "data_array") == false) {
                throw_exception("missing key \"data_array\" in input data");
            }
            if (input_data["data_array"].is_array() == false) {
                throw_exception("value of \"data_array\" field is not an array");
            }

            nlohmann::json input_data_array = input_data["data_array"];
            nlohmann::json output_data_array;
            if (onAction(input_data["action"], input_data_array, output_data_array) == false) {
                output_data["status"] = "failed";
                return;
            }
            /* Проверка ответа модуля */
            if (output_data_array.is_array() == false) {
                throw_exception("returned value is not an array");
            }
            output_data["status"] = "success";
            output_data["data_array"] = output_data_array;
            return;
        }
    }

    virtual bool onProcess(const nlohmann::json input_data_array, nlohmann::json& output_data_array) = 0;
    virtual bool onSetCallback(int64_t func_pointer) {
        _callback_func = reinterpret_cast<CallbackFunc>(func_pointer);
        registerController();
        loadData();
        return true;
    }
    virtual bool onAction(const std::string action, const nlohmann::json input_data_array, nlohmann::json& output_data_array) {
        UNUSED(action) UNUSED(input_data_array) UNUSED(output_data_array)
        throw_exception(action + " action called, but virtual function \"onAction\" not overrided");
    }

    /* Метод возвращает данные, которые были сохранены ранее методом writeData */
    nlohmann::json readData() {
        return _data_buffer;
    }

    /* Метод заменяет данные, которые были сохранены ранее, новыми, переданными в качестве аргумента        */
    /* Дозапись осуществляется путем вызова метода чтения, ручной модификацией данных, вызова метода записи */
    void writeData(nlohmann::json jsupdated_data) {
        _data_buffer = jsupdated_data;
        saveData();
    }

    /* Метод вызывается внутри реализации onProcess при каждом успешном получении результатов */
    void stashObject(std::string obj_name, nlohmann::json jsobj_value) {
        /* Формирование json-объекта, поля котрого содержат массивы однотипных объектов */
        _stashed_objects[obj_name].push_back(jsobj_value);
        /* Запсись полученного объекта в базу данных */
        nlohmann::json resulting_object;
        resulting_object[obj_name] = jsobj_value;
        saveResultingObject(resulting_object);
    }

    /* Метод вызывается внутри реализации onProcess при завршении обработки входных данных и формировании ответа */
    nlohmann::json dumpObjects() {
        /* Преобразование полей json-объекта в массив */
        nlohmann::json jsdump;
        for (auto& item : _stashed_objects.items()) {
            jsdump.push_back(item);
        }
        _stashed_objects.clear();
        return jsdump;
    }

private:

    /* Модуль может использовать постоянное внешнее хранилище данных, содержимое которого
     * хранится в произваольной форме. Запись и чтение происходит со всей информацией разом */
    void loadData() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _workname;
        request["action"] = "load";
        _callback_func(request, response);
        _data_buffer = response;
    }

    void saveData() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _workname;
        request["action"] = "save";
        request["data"] = _data_buffer;
        _callback_func(request, response);
    }

    void registerController() {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _workname;
        request["action"] = "register";
        _callback_func(request, response);
    }

    void saveResultingObject(nlohmann::json jsobject) {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _workname;
        request["action"] = "save_result";
        request["object"] = jsobject;
        _callback_func(request, response);
    }

private:

    /* Метаданные */
    ModuleType          _type;
    std::string         _name;
    std::string         _workname;
    std::string         _version;
    std::string         _implicit_dependence;
    std::string         _explicit_dependence;

    /* Связь от модуля к ядру */
    CallbackFunc        _callback_func;

    /* Вспомогательные члены класса */
    std::string         _json_dump_buffer;
    bool                _used;
    nlohmann::json      _stashed_objects;
    nlohmann::json      _data_buffer;

};
