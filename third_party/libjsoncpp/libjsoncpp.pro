# jsoncpp 0.6.0-rc2 amalgamation

CONFIG += static

TARGET = jsoncpp
TEMPLATE = lib

DEFINES += JSON_IS_AMALGAMATION

SOURCES += jsoncpp.cpp

QMAKE_CXXFLAGS_WARN_ON += \
        -Wno-type-limits \
        -Wno-unused-parameter \
        -Wno-extra