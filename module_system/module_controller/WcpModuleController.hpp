#pragma once
#include "../module_base/WcpAbstractModule.hpp"
#include <functional>
#include <unordered_map>

using ModuleList = std::list<WcpAbstractModule*>;
using ImageList = std::list<cv::Mat>;
/* Список саб-картинок и счетчик необработанных */
using SubImage = std::pair<uint64_t,ImageList>;
using SubImageList = std::unordered_map<uint64_t,SubImage>;
using ImageQueue = AsyncQueue<cv::Mat>;

typedef std::function<void(nlohmann::json,nlohmann::json&)> LoopFunction;

/* Класс принимает изображение, которое обрабатывается
 * множеством переданных ему модулей */
class WCP_DLL_EXPORT WcpModuleController
{

public:

    WcpModuleController(uint64_t controller_handler, CallbackFunc controller_callback);
    ~WcpModuleController();

    /* Метод добавляет модуль в кучу */
    void                add(WcpAbstractModule* module);

    /* Метод исключает модуль из кучи */
    void                remove(WcpAbstractModule* module);

    /* Метод пропускает изображение через граф модулей и возвращает массив результов их работы */
    void                processImage(cv::Mat* cvimage);

    /* Подписка на появление объекта по его имени */
    void                subscribeToObject(std::string object_name);


    void                sendCommand(WcpAbstractModule* module, nlohmann::json message);

    void                commitObject(nlohmann::json object);

    void                process(nlohmann::json message);


private:

    void                createCallbackFunc();
    void                startProcessing();
    void                stopProcessing();

    void                setCallbackFunc(WcpAbstractModule* module);
//    void                removeCallbackFunc();

    void                propagateObject(nlohmann::json object);

    cv::Mat*            addImage(uint64_t root_cvimage, cv::Mat cvimage);
    void                removeImage(uint64_t root_cvimage);

    void                replaceRoiWithImage(nlohmann::json& object);
    void                removeParentImage(nlohmann::json object);

    void                saveObject(nlohmann::json object);
    void                addObject(nlohmann::json object);
    void                notifyHandler(nlohmann::json object);

    void                sendCommandToHandler(std::string action, nlohmann::json data);

    void printTable();


private:

    uint64_t            _controller_handler;
    CallbackFunc        _controller_callback;

    StringList          _subscribe_object_list;


//    WcpModuleConnection* _connection;

    ModuleList          _module_list; /* Список всех модулей, участвующих в обработке изображения */

    ImageList           _image_list; /* Список исходных изображений */
    SubImageList        _sub_images; /* Список производных изображений */

    std::mutex          _data_mutex;
    nlohmann::json      _data_list;  /* Массив результатов обработки модулей */


    std::mutex          _heap_mutex;
    nlohmann::json      _heap;

    std::thread         _thread;
    std::atomic_bool    _running;

//    std::function<void(const char*)>  _callback_func;
    CallbackFunc _callback_func;

};
