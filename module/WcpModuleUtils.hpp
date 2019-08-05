#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

class WcpModuleUtils
{

public:

    /* Метод принимает cv::Mat изображение, возврщает base64 */
    static std::string encodeBase64Image(cv::Mat cvimg);

    /* Метод принимает base64 изображение из входного джейсона, возврщает cv::Mat */
    static cv::Mat decodeBase64Image(std::string encoded_image);

    /* Метод проверяет наличие ключа в переданном джейсоне */
    static bool keyExist(const nlohmann::json& js, std::string key);

};
