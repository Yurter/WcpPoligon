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

/* Класс описывает виртуальный интерфейс для наследующих его модулей */
class WCP_DLL_EXPORT WcpAbstractModule
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
//    { }
    {
        std::cout << __FUNCTION__ << std::endl;
    }

    WcpAbstractModule(WcpAbstractModule&) = delete;
    WcpAbstractModule(WcpAbstractModule&&) = delete;

    virtual ~WcpAbstractModule() = default;

    int                 type()                  const { return _type;                           }
    const char*         name()                  const { return _name.c_str();                   }
    const char*         version()               const { return _version.c_str();                }
    const char*         implicitDependence()    const { return _implicit_dependence.c_str();    }
    const char*         explicitDependence()    const { return _explicit_dependence.c_str();    }

    /* Си интерфейс для метода process(const nlohmann::json input_data, nlohmann::json& output_data) : void */
    [[nodiscard]] const char* process(const char* input_data)
    {
        nlohmann::json output_data;
        process(nlohmann::json::parse(input_data), output_data);
        return output_data.dump().c_str();
    }

protected:

    /* Метод, реализующий целевое назначение модуля */
    virtual void process(const nlohmann::json input_data, nlohmann::json& output_data) = 0;

private:

    ModuleType          _type;
    std::string         _name;
    std::string         _version;
    ModuleContext       _context_sensitive;
    std::string         _workdir;
    std::string         _implicit_dependence;
    std::string         _explicit_dependence;

};
