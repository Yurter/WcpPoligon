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

    try {
        { /* Основной цикл */
            while (cv::waitKey(5) != 'q') {
                source >> source_image;

                static int frame_counter = 0; /* Отладочный пропуск кадров для легкого чтения лога */
                frame_counter++;
                if (frame_counter % 40 == 0) {
                    auto result = module_controller.propagateImage(source_image);

                    std::cout << "===========================================" << std::endl;
                    for (auto&& elem : result) {
                        std::cout << "| " << elem.begin().key() << std::endl;
                    }
                    std::cout << "===========================================" << std::endl;
                }

                /* Увеличение картинки в два раза */
                resize(source_image, source_image, cv::Size(0,0), 2, 2, cv::INTER_CUBIC);
                cv::imshow("Source", source_image);
            }
        }
    } catch (std::exception e) {
        std::cout << "main: std::exception: " << e.what() << std::endl;
        return -1;
    }

    module_maneger.unload();

    return 0;

}
