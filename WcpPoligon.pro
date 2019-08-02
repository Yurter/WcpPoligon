TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
        module/WcpAbstractModule.cpp \
        module/WcpModuleWrapper.cpp \
        module_manager/WcpModuleHandler.cpp \
        module_manager/WcpModuleManager.cpp

HEADERS += \
    module/WcpAbstractModule.hpp \
    module/WcpModuleWrapper.hpp \
    module_manager/WcpModuleHandler.hpp \
    module_manager/WcpModuleManager.hpp

#nlohmann::json
INCLUDEPATH += D:\dev\
INCLUDEPATH += D:\dev\nlohmann

#OpenCV
INCLUDEPATH += D:\dev\00_opencv\410\build\include
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\lib
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\bin
LIBS += -lopencv_world410
