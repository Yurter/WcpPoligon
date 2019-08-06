#pragma once
#include "../module/WcpAbstractModule.hpp"

using ModuleList = std::list<WcpAbstractModule*>;

/* Класс составляет из переданных ему модулей граф, по которму
   распространяет результаты работы модулей от верхних к нижним */
class WcpModuleController
{

public:

    WcpModuleController() = default;
    ~WcpModuleController() = default;

    /* Метод добавляет модуль в кучу перед инициализацией графа */
    void                add(WcpAbstractModule* module);

    /* Метод выстраивает граф на основе зависимостей переданных ему модулей */
    bool                initGraph();

    /* Метод пропускает изображение через граф модулей и возвращает массив результов их работы */
    nlohmann::json      propagateImage(cv::Mat image);

//    nlohmann::json

private:

    nlohmann::json      recursion(nlohmann::json input_data);


private:

    ModuleList          _module_list;

};

