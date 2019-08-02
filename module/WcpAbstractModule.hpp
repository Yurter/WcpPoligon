#pragma once

#include <json.hpp>
#include <opencv2/core/core.hpp>

/* Тип модуля определяет область в которой определено его поведение */
enum ModuleType {
    MotionDetector,     /* Детекция движения на основе последовательности кадров        */
    ObjectDetector,     /* Детекция и классификация объектов на изображении             */
    Recognition,        /* Распознание объектов на изображении (лица, текст)            */
    Tracking,           /* Слежение за объектом в последовательности кадров на видео    */
    Utils,              /* Модуль модифицирует изображение, не сообщает метаинформации  */
    Logic               /* Сбор и анализ данных, полученных от других модулей           */
};

/* Зависимость модуля от контекста */
enum ModuleContext {
    ContextDependent,   /* Модулю необходимо хранить и обрабатывать информацию в конкретном контексте   */
    ContextIndependent  /* Модуль не зависит от контекста (от результатов предыдущих обращений)         */
};

class WcpAbstractModule
{

public:

    WcpAbstractModule(ModuleType type                   /* Реализуемый тип модуля                                       */
                      , std::string name                /* Имя модуля в произвольной форме, отображается пользователю   */
                      , std::string version             /* Версия модуля в формате чисел разделенных точками            */
                      , ModuleContext context_sensitive /* Требования модуля к индивидуальному контексту                */
                      , std::string workdir)  :         /* Директория, в которой модуль может найти необходимые файлы   */
        _type(type)
      , _name(name)
      , _version(version)
      , _context_sensitive(context_sensitive)
      , _workdir(workdir) { }

    virtual ~WcpAbstractModule() = default;

    ModuleType          type()      const { return _type; }
    std::string         name()      const { return _name; }
    std::string         version()   const { return _version; }

    virtual bool        process(const nlohmann::json input_data, nlohmann::json& output_data) = 0; /* Метод, реализующий целевое назначение модуля */

protected:

    cv::Mat             decodeBase64Image(std::string encoded_image);   /* Метод принимает base64 изображение из входного джейсона, возврщает cv::Mat */

private:

    ModuleType          _type;
    std::string         _name;
    std::string         _version;
    ModuleContext       _context_sensitive;
    std::string         _workdir;

};

