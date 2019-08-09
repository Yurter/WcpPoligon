#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>

using JsDataType = nlohmann::detail::value_t;

class WcpModuleUtils
{

public:

    /* Метод проверяет наличие ключа (key) в переданном nlohmann::json */
    static bool keyExist(const nlohmann::json& js, std::string key) {
        return js.find(key) != js.end();
    }

    /* Метод проверяет наличие и валидность поля с ключом key */
    static bool ckeckJsonField(const nlohmann::json& js, std::string key, JsDataType type) {
        if (keyExist(js, key) == false) { return false; }
        if (js[key].type() != type)     { return false; }
        return true;
    }

    /* Метод собирает nlohmann::json из cv::Mat */
//    static nlohmann::json imageToJson(cv::Mat& cvimage) {
//        std::vector<uchar> data_array;

//        if (cv::imencode(".png", cvimage, data_array) == false) {
//            throw std::exception("imageToJson failed");
//        }

//        nlohmann::json jscvimage;
//        jscvimage["image"]["data"] = data_array;

//        return jscvimage;
//    }

//    /* Метод собирает cv::Mat из nlohmann::json */
//    static cv::Mat jsonToImage(const nlohmann::json& jscvimage) {
//        std::vector<uchar> data = jscvimage["data"];
//        cv::Mat cvimage = cv::imdecode(data, 1);
//        return cvimage;
//    }

    /* Метод собирает джейсон из cv::Rect */
    template<typename cvRect>
    static nlohmann::json rectToJson(cvRect cvrect) {
        nlohmann::json jsrect;

        jsrect["rect"]["x"] = cvrect.x;
        jsrect["rect"]["y"] = cvrect.y;
        jsrect["rect"]["w"] = cvrect.width;
        jsrect["rect"]["h"] = cvrect.height;

        return jsrect;
    }

    /* Метод собирает cv::Rect из nlohmann::json */
    template<typename cvRect>
    static cvRect jsonToRect(nlohmann::json jsrect) {
        cvRect cvrect;

        cvrect.x = jsrect["rect"]["x"];
        cvrect.y = jsrect["rect"]["y"];
        cvrect.width  = jsrect["rect"]["w"];
        cvrect.height = jsrect["rect"]["h"];

        return cvrect;
    }

    /* Метод собирает джейсон из cv::Rect */
    static nlohmann::json createJsonObject(std::string key, nlohmann::json value) {
        nlohmann::json jsobject;
        jsobject[key] = value;
        return jsobject;
    }

    /* ? */
    static nlohmann::json imageToJson(cv::Mat& cvimage) {
        nlohmann::json jsimage;
        jsimage["image"] = reinterpret_cast<uint64_t>(&cvimage);
        return jsimage;
    }

    /* Метод конвертирует изображение из openCV в Dlib */
//    static dlib::matrix<rgb_pixel> cv2dlib(cv::Mat& cvimage) {
//        dlib::matrix<rgb_pixel> dlib_image;
//        dlib::assign_image(dlib_image, dlib::cv_image<bgr_pixel>(cvimage));
//        return dlib_image;
//    }

    /* Метод конвертирует изображение из Dlib в openCV */
//    static cv::Mat cv2dlib(dlib::matrix<rgb_pixel>& cvimage) {
//        //
//    }

};
