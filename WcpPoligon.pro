TEMPLATE = app
#TEMPLATE = lib
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        module_system/module_connection/WcpModuleConnection.cpp \
        module_system/module_controller/WcpModuleController.cpp \
        main.cpp \
        module_system/module_manager/WcpModuleHandler.cpp \
        module_system/module_manager/WcpModuleManager.cpp

HEADERS += \
    module_system/module_utils/AsyncQueue.hpp \
    module_system/module_base/WcpModuleHeader.hpp \
    module_system/module_utils/WcpModuleUtils.hpp \
    module_system/module_connection/WcpModuleConnection.hpp \
    module_system/module_controller/WcpModuleController.hpp \
    module_system/module_base/WcpAbstractModule.hpp \
    module_system/module_manager/WcpModuleHandler.hpp \
    module_system/module_manager/WcpModuleManager.hpp \
    module_system/WcpHeader.hpp

#nlohmann::json
INCLUDEPATH += D:\dev\
INCLUDEPATH += D:\dev\nlohmann

#OpenCV
INCLUDEPATH += D:\dev\00_opencv\410\build\include
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\lib
LIBS += -LD:\dev\00_opencv\410\build\x64\vc15\bin
LIBS += -lopencv_world410   #release
LIBS += -lopencv_world410d  #debug

DEFINES += WCP_MAIN
