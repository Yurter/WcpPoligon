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

/* Зависимость модуля от контекста */
enum ModuleContext {
    ContextDependent,   /* Модулю необходимо хранить и обрабатывать информацию в конкретном контексте   */
    ContextIndependent  /* Модуль не зависит от контекста (от результатов предыдущих обращений)         */
};

//using saveData = void(*)(nlohmann::json request, nlohmann::json& response);   /* ? */
//using loadData = void(*)(nlohmann::json request, nlohmann::json& response);   /* ? */

using CallbackFunc = void(*)(nlohmann::json request, nlohmann::json& response);   /* ? */

#define throw_exception(msg) throw std::exception(std::string(_name + " : " + std::string(msg)).c_str())

/* Класс описывает виртуальный интерфейс для наследующих его модулей */
class /*WCP_DLL_EXPORT*/ WcpAbstractModule
{

public:

    WcpAbstractModule(ModuleType type                   /* Реализуемый тип модуля                                       */
                      , std::string name                /* Имя модуля в произвольной форме, отображается пользователю   */
                      , std::string version             /* Версия модуля в формате чисел разделенных точками            */
                      , ModuleContext context_sensitive /* Требования модуля к индивидуальному контексту                */
                      , std::string workdir             /* Директория, в которой модуль может найти необходимые файлы   */
                      , std::string implicit_dependence /* Директория, в которой модуль может найти необходимые файлы   */
                      , std::string explicit_dependence /* Директория, в которой модуль может найти необходимые файлы   */
                      ) :
        _type(type)
      , _name(name)
      , _version(version)
      , _context_sensitive(context_sensitive)
      , _workdir(workdir)
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
    [[nodiscard]] const char* process(const char* input_data)
    {
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
    void process(const nlohmann::json input_data, nlohmann::json& output_data)
    {
        /* Проверка наличия полей с определенными ключами */
        if (WcpModuleUtils::keyExist(input_data, "action") == false) {
            throw_exception("missing key \"action\" in input data");
        }
        if (WcpModuleUtils::keyExist(input_data, "data_array") == false) {
            throw_exception("missing key \"data_array\" in input data");
        }
        /* Проверка типа полей */
        if (input_data["action"].is_string() == false) {
            throw_exception("value of \"action\" field is not a string");
        }
        if (input_data["data_array"].is_array() == false) {
            throw_exception("value of \"data_array\" field is not an array");
        }

        /* При предусмотренном типе действия, происходит вызов соответствующего метода */
        if (input_data["action"] == "process") {
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
            int64_t func_pointer = static_cast<int64_t>(input_data["callback_func"]);
            _callback_func = reinterpret_cast<CallbackFunc>(func_pointer);
            return;
        }

        /* При непредусмотренном типе действия, происходит вызов универсального метода */
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

    virtual bool onProcess(const nlohmann::json input_data_array, nlohmann::json& output_data_array) = 0;
    virtual bool onAction(const std::string action, const nlohmann::json input_data_array, nlohmann::json& output_data_array)
    {
        throw_exception(action + " action called, but virtual function \"onAction\" not overrided");
    }

    /* Модуль может использовать постоянное внешнее хранилище данных, содержимое которого
     * хранится в произваольной форме. Запись и чтение происходит со всей информацией разом */
    void saveData(nlohmann::json data)
    {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["action"] = "save";
        request["data"] = data;
        _callback_func(request, response);
    }
    nlohmann::json loadData()
    {
        if (_callback_func == nullptr) {
            throw_exception("Callback function is null");
        }
        nlohmann::json request;
        nlohmann::json response;
        request["action"] = "load";
        _callback_func(request, response);
        return response;
    }

private:

    ModuleType          _type;
    std::string         _name;
    std::string         _version;
    ModuleContext       _context_sensitive;
    std::string         _workdir;
    std::string         _implicit_dependence;
    std::string         _explicit_dependence;

    CallbackFunc        _callback_func;

    std::string         _json_dump_buffer;
    bool                _used;

};
