#pragma once
#include "../module/WcpAbstractModule.hpp"
#include <functional>
#include <unordered_map>

using ModuleList = std::list<WcpAbstractModule*>;
using ImageList = std::list<cv::Mat>;
using SubImageList = std::unordered_map<cv::Mat*,ImageList>;
using ImageQueue = AsyncQueue<cv::Mat>;

typedef std::function<void(nlohmann::json,nlohmann::json&)> LoopFunction;

/* Класс принимает изображение, которое обрабатывается
 * множеством переданных ему модулей */
class WCP_DLL_EXPORT WcpModuleController
{

public:

    WcpModuleController();
    ~WcpModuleController();

    /* Метод добавляет модуль в кучу */
    void                add(WcpAbstractModule* module);

    /* Метод исключает модуль из кучи */
//    void                remove(WcpAbstractModule* module);

    /* Метод пропускает изображение через граф модулей и возвращает массив результов их работы */
    void                processImage(cv::Mat* cvimage);


    void                sendCommand(WcpAbstractModule* module, nlohmann::json message);

    void                addObject(nlohmann::json object);


private:

    void                createCallbackFunc();
    void                startProcessing();
    void                stopProcessing();

    void                setCallbackFunc(WcpAbstractModule* module);
    void                removeCallbackFunc();

    void                propagateObject(nlohmann::json object);

    cv::Mat*            addImage(cv::Mat* parent_cvimage, cv::Mat cvimage);
    void                removeImage(cv::Mat* parent_cvimage, cv::Mat& cvimage);

    void                replaceRectWithImage(nlohmann::json& object);


private:

    ModuleList          _module_list; /* Список всех модулей, участвующих в обработке изображения */

    ImageList           _image_list; /* Список исходных изображений */
    SubImageList        _sub_images; /* Список производных изображений */

    std::mutex          _data_mutex;
    nlohmann::json      _data_list;  /* Массив результатов обработки модулей */


    std::mutex          _heap_mutex;
    nlohmann::json      _heap;

    std::thread         _thread;
    std::atomic_bool    _running;

    std::function<void(uint64_t,uint64_t,const char*)>  _callback_func;

};
