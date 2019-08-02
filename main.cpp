#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include "module_manager/WcpModuleManager.hpp"

#define WEB_CAMERA 0

int main()
{

    cv::VideoCapture source;
    source.open(WEB_CAMERA);
    cv::Mat source_image;

    WcpModuleManager module_maneger("D:\\dev\\WcpPoligon\\plugins");
    for (auto&& module : module_maneger.availableModules()) {
        std::cout << "available module: " << module << std::endl;
    }
    std::cout << std::endl;

    try {
        module_maneger.load();
    } catch (std::exception e) { std::cout << e.what() << std::endl; return -1; }

    { /* Основной цикл */
        while (cv::waitKey(5) != 'q') {
            source >> source_image;

            //

            /* Увеличение картинки в два раза */
            resize(source_image, source_image, cv::Size(0,0), 2, 2, cv::INTER_CUBIC);
            cv::imshow("Source", source_image);
        }
    }

    module_maneger.unload();

    return 0;

}
