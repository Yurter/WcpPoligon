#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

class WcpModuleUtils
{

public:

    /* Метод проверяет наличие ключа в переданном джейсоне */
    static bool keyExist(const nlohmann::json& js, std::string key)
    {
        return js.find(key) != js.end();
    }

    /* Метод собирает джейсон из cv картинки */
    static nlohmann::json imageToJson(cv::Mat& cvimage)
    {
        nlohmann::json jscvimage;

        std::vector<uchar> data_array;
        bool ret = cv::imencode(".png", cvimage, data_array);

        jscvimage["width"] = cvimage.cols;
        jscvimage["height"] = cvimage.rows;
        jscvimage["type"] = cvimage.type();
        jscvimage["data"] = data_array;

        return jscvimage;
    }

    /* Метод собирает cv картинку из джейсона */
    static cv::Mat jsonToImage(const nlohmann::json& jscvimage)
    {
        std::vector<uchar> data = jscvimage["data"];
        cv::Mat cvimage = cv::imdecode(data, 1);
        return cvimage;
    }

};
