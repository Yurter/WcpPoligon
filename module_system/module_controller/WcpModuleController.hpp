#pragma once
#include "../module/WcpAbstractModule.hpp"

using ModuleList = std::list<WcpAbstractModule*>;
using ImageList = std::list<cv::Mat>;
using ImageQueue = AsyncQueue<cv::Mat>;

/* Класс принимает изображение, которое обрабатывается
 * множеством переданных ему модулей */
class WcpModuleController
{

public:

    WcpModuleController();
    ~WcpModuleController();

    /* Метод добавляет модуль в кучу */
    void                add(WcpAbstractModule* module);

    /* Метод исключает модуль из кучи */
//    void                remove(WcpAbstractModule* module);

    /* Метод пропускает изображение через граф модулей и возвращает массив результов их работы */
    void                processImage(cv::Mat cvimage);


private:

    /* Метод устанавливает всем модулям сallback-функцию */
    void                setCallbackFunc(WcpAbstractModule* module);

    void                propagateObject(nlohmann::json object);

    void                processRecursively(nlohmann::json object_row);

    void                callbackFunc(nlohmann::json request, nlohmann::json& response);

private:

    ModuleList          _module_list; /* Список всех модулей, участвующих в обработке изображения */

    std::mutex          _data_mutex;
    nlohmann::json      _data_list;  /* Массив результатов обработки модулей */
    ImageList           _image_list; /* Список обрабатываемых изображений */

    std::mutex          _heap_mutex;
    nlohmann::json      _heap;

    std::thread         _thread;
    volatile bool       _running;

    CallbackFunc        _callback_func;

};
