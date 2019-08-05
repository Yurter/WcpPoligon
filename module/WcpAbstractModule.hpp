#pragma once
#include <json.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#define WCP_DLL_EXPORT __declspec(dllexport)

/* Тип модуля определяет область в которой определено его поведение */
enum ModuleType {
    MotionDetector,     /* Детекция движения на основе последовательности кадров        */
    ObjectDetector,     /* Детекция и классификация объектов на изображении             */
    Recognition,        /* Распознание объектов на изображении (лица, текст)            */
    Tracking,           /* Слежение за объектом в последовательности кадров на видео    */
    Utils,              /* Модуль модифицирует изображение, не сообщает метаинформации  */
    Logic               /* Сбор и анализ данных, полученных от других модулей           */
};

/* Зависимость модуля от контекста */
enum ModuleContext {
    ContextDependent,   /* Модулю необходимо хранить и обрабатывать информацию в конкретном контексте   */
    ContextIndependent  /* Модуль не зависит от контекста (от результатов предыдущих обращений)         */
};

class WCP_DLL_EXPORT WcpAbstractModule
{

public:

    WcpAbstractModule(ModuleType type                   /* Реализуемый тип модуля                                       */
                      , std::string name                /* Имя модуля в произвольной форме, отображается пользователю   */
                      , std::string version             /* Версия модуля в формате чисел разделенных точками            */
                      , ModuleContext context_sensitive /* Требования модуля к индивидуальному контексту                */
                      , std::string workdir)  :         /* Директория, в которой модуль может найти необходимые файлы   */
        _type(type)
      , _name(name)
      , _version(version)
      , _context_sensitive(context_sensitive)
      , _workdir(workdir) { }

    WcpAbstractModule(WcpAbstractModule&) = delete;
    WcpAbstractModule(WcpAbstractModule&&) = delete;

    virtual ~WcpAbstractModule() = default;

    int                 type()      const { return _type;               }
    const char*         name()      const { return _name.c_str();       }
    const char*         version()   const { return _version.c_str();    }

    const char*         process(const char* input_data)
    {
        nlohmann::json output_data;
        process(input_data, output_data);
        return output_data.dump().c_str();
    }

protected:

    /* Метод, реализующий целевое назначение модуля */
    virtual void process(const nlohmann::json input_data, nlohmann::json& output_data) = 0;

    /* Метод принимает cv::Mat изображение, возврщает base64 */
    std::string encodeBase64Image(cv::Mat cvimg);

    /* Метод принимает base64 изображение из входного джейсона, возврщает cv::Mat */
    cv::Mat decodeBase64Image(std::string encoded_image)
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

    bool keyExist(const nlohmann::json& js, std::string key)
    {
        return js.find(key) != js.end();
    }

private:

    ModuleType          _type;
    std::string         _name;
    std::string         _version;
    ModuleContext       _context_sensitive;
    std::string         _workdir;

    const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

};
