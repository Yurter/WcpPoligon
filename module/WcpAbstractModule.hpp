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

using StringList = std::list<std::string>;

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
                      , StringList  result_object_list  /* ? */
                      ) :
        _type(type)
      , _name(name)
      , _workname(workname)
      , _version(version)
      , _implicit_dependence(implicit_dependence)
      , _explicit_dependence(explicit_dependence)
      , _result_object_list(result_object_list)
      , _callback_func(nullptr)
      , _objects_uid(0)
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
     *      "action"     : String,    ex.: "process"
     *      "object_array2d" : Array      ex. 1: [ face_0, face_1, face_2, ... face_N ]   Распознание лиц
     *                                ex. 2: [ car_0, car_1, car_2, ... car_N ]       Распознание автомобильных номеров
     * }
     * * На выход:
     * {
     *      "status"     : String,    ex.: "success"/"failed"
     *      "object_array2d" : Array      ex. 1: [ "cat" : cat_array, "dog" : dog_array, ... "smth" : smth_array ] Мультиобъектный детектор
     *                                ex. 2: [ "face" : face_array ]                                           Монообъектный детектор
     * }
    **/
    void process(const nlohmann::json input_data, nlohmann::json& output_data) {

        if (WcpModuleUtils::ckeckJsonField(input_data, "action", JsDataType::string) == false) {
            throw_exception("invalid or missing \"action\" field in input data");
        }

        /* При предусмотренном типе действия, происходит вызов соответствующего метода */
        if (input_data["action"] == "process") {
            if (WcpModuleUtils::ckeckJsonField(input_data, "object_array1d", JsDataType::array) == false) {
                throw_exception("invalid or missing \"object_array1d\" field in input data");
            }

            nlohmann::json input_object_array1d = input_data["object_array1d"];
            nlohmann::json output_object_array2d;
            onProcess(input_object_array1d);

            output_object_array2d = dumpObjects();

            if (output_object_array2d.empty()) {
                output_data["status"] = "failed";
                return;
            }

            output_data["status"] = "success";
            output_data["object_array2d"] = output_object_array2d;
            return;
        }

        if (input_data["action"] == "set_callback") {
            if (WcpModuleUtils::ckeckJsonField(input_data, "callback_func", JsDataType::number_unsigned) == false) {
                throw_exception("invalid or missing \"callback_func\" field in input data");
            }

            onSetCallback(input_data["callback_func"]);
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

    virtual void onProcess(const nlohmann::json input_object_array2d) = 0;
    virtual void onSetCallback(uint64_t func_pointer) {
        _callback_func = reinterpret_cast<CallbackFunc>(func_pointer);
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
        saveResultingObject(resulting_object);
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
        request["module"] = _workname;
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
        request["module"] = _workname;
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
        request["module"] = _workname;
        request["action"] = "register";
        request["object_list"] = _result_object_list;
        _callback_func(request, response);

        if (WcpModuleUtils::ckeckJsonField(response, "objects_uid", JsDataType::array) == false) {
            throw_exception("invalid or missing \"objects_uid\" field in response");
        }
        auto objects_uid_array = response["objects_uid"];
        _objects_uid = WcpModuleUtils::arrayToObject(objects_uid_array);
    }

    void saveResultingObject(nlohmann::json jsobject) {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["module"] = _workname;
        request["action"] = "save_object";
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
    StringList          _result_object_list;

    /* Вспомогательные члены класса */
    CallbackFunc        _callback_func;     /* Связь от модуля к ядру                                               */
    nlohmann::json      _objects_uid;       /* UID объекта в контексте моудуля                                      */
    std::string         _json_dump_buffer;  /* Возвращаемый указатель метода process(const char* input_data)        *
                                             * ссылается на содержмиое этой строки                                  */
    bool                _used;              /* Флаг, не позволяющий повтроное использование модуля в контексте      *
                                             * одной итерации обработки изображения                                 */
    nlohmann::json      _stashed_objects;   /* Буффер результирующих объектов                                       */
    nlohmann::json      _module_data;       /* Копия произвольных данных модуля, хранящихся во внешнем хранилище    */

};
