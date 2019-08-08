#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

class WcpModuleUtils
{

public:

    /* Метод проверяет наличие ключа (key) в переданном nlohmann::json */
    static bool keyExist(const nlohmann::json& js, std::string key)
    {
        return js.find(key) != js.end();
    }

    /* Метод собирает nlohmann::json из cv::Mat */
    static nlohmann::json imageToJson(cv::Mat& cvimage)
    {
        std::vector<uchar> data_array;
        bool ret = cv::imencode(".png", cvimage, data_array);

        nlohmann::json jscvimage;
        jscvimage["image"]["data"] = data_array;

        return jscvimage;
    }

    /* Метод собирает cv::Mat из nlohmann::json */
    static cv::Mat jsonToImage(const nlohmann::json& jscvimage)
    {
        std::vector<uchar> data = jscvimage["data"];
        cv::Mat cvimage = cv::imdecode(data, 1);
        return cvimage;
    }

    /* Метод собирает джейсон из cv::Rect */
    template<typename cvRect>
    static nlohmann::json rectToJson(cvRect cvrect)
    {
        nlohmann::json jsrect;

        jsrect["rect"]["x"] = cvrect.x;
        jsrect["rect"]["y"] = cvrect.y;
        jsrect["rect"]["w"] = cvrect.width;
        jsrect["rect"]["h"] = cvrect.height;

        return jsrect;
    }

    /* Метод собирает cv::Rect из nlohmann::json */
    template<typename cvRect>
    static cvRect jsonToRect(nlohmann::json jsrect)
    {
        cvRect cvrect;

        cvrect.x = jsrect["rect"]["x"];
        cvrect.y = jsrect["rect"]["y"];
        cvrect.width  = jsrect["rect"]["w"];
        cvrect.height = jsrect["rect"]["h"];

        return cvrect;
    }

    /* Метод собирает джейсон из cv::Rect */
    static nlohmann::json createJsonObject(std::string key, nlohmann::json value)
    {
        nlohmann::json jsobject;
        jsobject[key] = value;
        return jsobject;
    }

    /* ? */
//    static nlohmann::json dosmth()
//    {
//        //
//    }

    /* ? */
//    static nlohmann::json concatJsonObjects(nlohmann::json json_list)
//    {
//        std::cout << "json_list = " << json_list << std::endl;
//        nlohmann::json jsresult;// = nlohmann::json().flatten();
//        for (auto&& elem : json_list) {
//            std::cout << "elem = " << elem << std::endl;
//            for (auto it = elem.begin(); it != elem.end(); ++it) {
//                jsresult[it.key()] = it.value();
//            }
//        }
//        return jsresult;
//    }

};
