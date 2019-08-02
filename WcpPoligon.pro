TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        module/WcpAbstractModule.cpp \
        module/WcpModuleWrapper.cpp

HEADERS += \
    module/WcpAbstractModule.hpp \
    module/WcpModuleWrapper.hpp

#nlohmann::json
INCLUDEPATH += D:\dev\
INCLUDEPATH += D:\dev\nlohmann

#OpenCV
INCLUDEPATH += D:\dev\00_opencv\410\build\include
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\lib
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\bin
LIBS += -lopencv_world410
