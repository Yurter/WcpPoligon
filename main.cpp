#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "module_manager/WcpModuleManager.hpp"
#include "module_controller/WcpModuleController.hpp"

#define WEB_CAMERA 0

int main()
{

    /* Открытие веб-камеры на чтение */
    cv::VideoCapture source;
    source.open(WEB_CAMERA);
    cv::Mat source_image;

    /* Инициализация менеджера модулей */
    WcpModuleManager module_maneger("D:\\dev\\WcpPoligon\\plugins");
    for (auto&& module : module_maneger.availableModules()) {
        std::cout << "available module: " << module << std::endl;
    }
    std::cout << std::endl;
    try {
        module_maneger.load();
    } catch (std::exception e) { std::cout << e.what() << std::endl; return -1; }

    /* Построение графа из модулей для текущей камеры */
    WcpModuleController module_controller;
    for (auto&& module_handler : *module_maneger.handlerList()) {
        module_controller.add(module_handler.module());
    }
    if (!module_controller.initGraph()) {
        std::cout << "Failed to init module controller' graph" << std::endl;
        return -1;
    }

    { /* Основной цикл */
        while (cv::waitKey(5) != 'q') {
            source >> source_image;

            auto result = module_controller.propagateImage(source_image);
            break;

            /* Увеличение картинки в два раза */
            resize(source_image, source_image, cv::Size(0,0), 2, 2, cv::INTER_CUBIC);
            cv::imshow("Source", source_image);
        }
    }

    module_maneger.unload();

    return 0;

}
