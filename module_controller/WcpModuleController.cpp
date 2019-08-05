#include "WcpModuleController.hpp"
#include "../module/WcpModuleUtils.hpp"
#include <iostream>

void WcpModuleController::add(WcpAbstractModule* module)
{
    _module_list.push_back(module);
}

bool WcpModuleController::initGraph()
{
    return true;
}

nlohmann::json WcpModuleController::propagateImage(cv::Mat image)
{
    cv::imwrite("Debug3.png", image);
    nlohmann::json output_json;
    //temp solution
    nlohmann::json input_data;
    input_data["action"] = "detect";
    if (WcpModuleUtils::keyExist(input_data, "action") == false) {
        std::cout << "[Debug] action does not exist" << std::endl;
        return nlohmann::json();
    }
    std::cout << "[Debug] input_data = " << input_data << std::endl;
    std::cout << "[Debug] image type = " << image.type() << " " << CV_8UC1 << std::endl;
    std::cout << "[Debug] image size = " << image.cols << "x"<< image.rows << "=" << image.cols * image.rows << std::endl;
//    std::cout << "[Debug] image = " << image << std::endl;
//    std::cout << "[Debug] WcpModuleUtils::encodeBase64Image(image) = " << WcpModuleUtils::encodeBase64Image(image) << std::endl;
//    input_data["image"].push_back({
//                                      { "width", image.cols }
//                                      , { "height", image.rows }
//                                      , { "data", WcpModuleUtils::encodeBase64Image(image)
//                                      }
//                                  });

    std::vector<uchar> array2;
    if (image.isContinuous()) {
      array2.assign((uchar*)image.datastart, (uchar*)image.dataend);
    } else {
      for (int i = 0; i < image.rows; ++i) {
        array2.insert(array2.end(), image.ptr<uchar>(i), image.ptr<uchar>(i)+image.cols);
      }
    }

    input_data["image"]["width"] = image.cols;
    input_data["image"]["height"] = image.rows;
//    input_data["image"]["data"] = WcpModuleUtils::encodeBase64Image(image);
//    input_data["image"]["data"] = std::vector<uchar>(image.data, image.data + image.cols * image.rows);
    input_data["image"]["type"] = image.type();
    input_data["image"]["data"] = array2;






    std::vector<uchar> test_arr = input_data["image"]["data"];

    std::vector<uchar> test_data = input_data["image"]["data"];
    cv::Mat source_cvimg(input_data["image"]["height"]
            , input_data["image"]["width"]
            , input_data["image"]["type"] //CV_8U
            , test_arr.data());
    cv::imwrite("Debug2.png", source_cvimg);
//    return 0;


    std::cout << "[Debug] std::vector<uchar>(image.data, image.data + image.cols * image.rows) = " << std::vector<uchar>(image.data, image.data + image.cols * image.rows).size() << std::endl;
//    input_data["image"] = WcpModuleUtils::encodeBase64Image(image);
    for (auto&& module : _module_list) {
        auto ret = module->process(input_data.dump().c_str());
        auto module_answer = nlohmann::json::parse(ret);
        std::cout << "[Debug] module_answer = " << module_answer << std::endl;
        output_json.push_back({ module->name(), module_answer });
    }
    return output_json;
}
