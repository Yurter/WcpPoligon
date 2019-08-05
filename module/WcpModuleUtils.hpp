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
    static std::string encodeBase64Image(cv::Mat cvimg)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        uchar const* bytes_to_encode = cvimg.data;
        int in_len = cvimg.cols * cvimg.rows;

        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i <4) ; i++) {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }

        if (i) {
            for(j = i; j < 3; j++) {
                char_array_3[j] = '\0';
            }

            char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; (j < i + 1); j++) {
                ret += base64_chars[char_array_4[j]];
            }

            while((i++ < 3)) {
                ret += '=';
            }
        }

        return ret;
    }

    /* Метод принимает base64 изображение из входного джейсона, возврщает cv::Mat */
    static cv::Mat decodeBase64Image(std::string encoded_image)
    {
        size_t in_len = encoded_image.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string decoded_string;

        auto isBase64 = [](unsigned char c) { return (isalnum(c) || (c == '+') || (c == '/')); };

        while (in_len-- && (encoded_image[in_] != '=') && isBase64(encoded_image[in_])) {
            char_array_4[i++] = encoded_image[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    decoded_string += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) decoded_string += char_array_3[j];
        }

        std::vector<uchar> data(decoded_string.begin(), decoded_string.end());
        cv::Mat cvimg = imdecode(data, cv::IMREAD_UNCHANGED);
        return cvimg;
    }

    /* Метод проверяет наличие ключа в переданном джейсоне */
    static bool keyExist(const nlohmann::json& js, std::string key)
    {
        return js.find(key) != js.end();
    }

};
