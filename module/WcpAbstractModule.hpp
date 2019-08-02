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

    WcpAbstractModule(ModuleType type, std::string name, std::string version, ModuleContext context_sensitive, std::string workdir);
    virtual ~WcpAbstractModule() = 0;

    ModuleType          type()      const;
    std::string         name()      const;
    std::string         version()   const;

    virtual bool        process(const nlohmann::json input_data, nlohmann::json& output_data) = 0;

protected:

    cv::Mat             decodeBase64Image(std::string encoded_image);

private:

    ModuleType          _type;
    std::string         _name;
    std::string         _version;
    ModuleContext       _context_sensitive;
    std::string         _workdir;

};

