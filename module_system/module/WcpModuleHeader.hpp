#pragma once
#include <string>
#include <list>

/* Тип модуля определяет область в которой определено его поведение */
enum ModuleType {
    MotionDetector,     /* Детекция движения на основе последовательности кадров        */
    ObjectDetector,     /* Детекция и классификация объектов на изображении             */
    Recognition,        /* Распознание объектов на изображении (лица, текст)            */
    Tracking,           /* Слежение за объектом в последовательности кадров на видео    */
    Helper,             /* Модуль модифицирует изображение, не сообщает метаинформации  */
    Logic               /* Сбор и анализ данных, полученных от других модулей           */
};

using StringList = std::list<std::string>;

/* Класс содержит в себе метаиинформацию о модуле */
class WcpModuleHeader
{

public:

    WcpModuleHeader(ModuleType type                   /* Реализуемый тип модуля                                       */
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
    { }

    ~WcpModuleHeader();

    int                 type()                  const { return _type;                           }
    const char*         name()                  const { return _name.c_str();                   }
    const char*         workname()              const { return _workname.c_str();               }
    const char*         version()               const { return _version.c_str();                }
    const char*         implicitDependence()    const { return _implicit_dependence.c_str();    }
    const char*         explicitDependence()    const { return _explicit_dependence.c_str();    }
    StringList          resultObjectList()      const { return _result_object_list;             }


private:

    /* Метаданные */
    ModuleType          _type;
    std::string         _name;
    std::string         _workname;
    std::string         _version;
    std::string         _implicit_dependence;
    std::string         _explicit_dependence;
    StringList          _result_object_list;

};

