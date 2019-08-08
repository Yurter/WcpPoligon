#pragma once
#include "../module/WcpAbstractModule.hpp"

using ModuleList = std::list<WcpAbstractModule*>;

/* Класс принимает изображение, которое обрабатывается
 * множеством переданных ему модулей */
class WcpModuleController
{

public:

    WcpModuleController() = default;
    ~WcpModuleController() = default;

    /* Метод добавляет модуль в кучу перед инициализацией графа */
    void                add(WcpAbstractModule* module);

    /* Метод пропускает изображение через граф модулей и возвращает массив результов их работы */
    nlohmann::json      propagateImage(cv::Mat image);

private:

    void                recursion();


private:

    ModuleList          _module_list;

    nlohmann::json      _data_list;

};

