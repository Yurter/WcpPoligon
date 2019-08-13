TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        module_system/module_controller/WcpModuleController.cpp \
        main.cpp \
        module_system/module_manager/WcpModuleHandler.cpp \
        module_system/module_manager/WcpModuleManager.cpp

HEADERS += \
    module_system/module/AsyncQueue.hpp \
    module_system/module/WcpModuleHeader.hpp \
    module_system/module/WcpModuleUtils.hpp \
    module_system/module_controller/WcpModuleController.hpp \
    module_system/module/WcpAbstractModule.hpp \
    module_system/module_manager/WcpModuleHandler.hpp \
    module_system/module_manager/WcpModuleManager.hpp

#nlohmann::json
INCLUDEPATH += D:\dev\
INCLUDEPATH += D:\dev\nlohmann

#OpenCV
INCLUDEPATH += D:\dev\00_opencv\410\build\include
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\lib
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\bin
LIBS += -lopencv_world410   #release
LIBS += -lopencv_world410d  #debug
