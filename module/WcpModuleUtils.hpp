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
        if (cvimage.isContinuous()) {
            data_array.assign(const_cast<uchar*>(cvimage.datastart)
                              , const_cast<uchar*>(cvimage.dataend));
        } else {
            for (int i = 0; i < cvimage.rows; ++i) {
                data_array.insert(data_array.end()
                                  , cvimage.ptr<uchar>(i)
                                  , cvimage.ptr<uchar>(i) + cvimage.cols);
            }
        }

//        jscvimage["image"]["width"] = cvimage.cols;
//        jscvimage["image"]["height"] = cvimage.rows;
//        jscvimage["image"]["type"] = cvimage.type();
//        jscvimage["image"]["data"] = data_array;
        jscvimage["width"] = cvimage.cols;
        jscvimage["height"] = cvimage.rows;
        jscvimage["type"] = cvimage.type();
        jscvimage["data"] = data_array;

        return jscvimage;
    }

    /* Метод собирает cv картинку из джейсона */
    static cv::Mat jsonToImage(const nlohmann::json& jscvimage)
    {
//        std::vector<uchar> data = jscvimage["image"]["data"];

//        cv::Mat cvimage(jscvimage["image"]["height"]
//                , jscvimage["image"]["width"]
//                , jscvimage["image"]["type"]
//                , data.data());
        std::vector<uchar> data = jscvimage["data"];

        cv::Mat cvimage(jscvimage["height"]
                , jscvimage["width"]
                , jscvimage["type"]
                , data.data());

        return cvimage;
    }

};
