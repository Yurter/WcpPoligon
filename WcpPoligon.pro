TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        module_controller/WcpModuleController.cpp \
        main.cpp \
        module_manager/WcpModuleHandler.cpp \
        module_manager/WcpModuleManager.cpp

HEADERS += \
    module_controller/WcpModuleController.hpp \
    module/WcpAbstractModule.hpp \
    module_manager/WcpModuleHandler.hpp \
    module_manager/WcpModuleManager.hpp

#nlohmann::json
INCLUDEPATH += D:\dev\
INCLUDEPATH += D:\dev\nlohmann

#OpenCV
INCLUDEPATH += D:\dev\00_opencv\410\build\include
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\lib
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\bin
LIBS += -lopencv_world410   #release
LIBS += -lopencv_world410d  #debug
