TEMPLATE = lib
TARGET = qtllm
CONFIG += c++17 warn_on
QT += core network

DEFINES += QTLLM_LIBRARY

INCLUDEPATH += \
    $$PWD \
    $$PWD/core \
    $$PWD/providers \
    $$PWD/network \
    $$PWD/streaming

HEADERS += \
    core/llmconfig.h \
    core/llmtypes.h \
    core/qtllmclient.h \
    providers/illmprovider.h \
    providers/openaicompatibleprovider.h \
    providers/ollamaprovider.h \
    providers/vllmprovider.h \
    network/httpexecutor.h \
    streaming/streamchunkparser.h

SOURCES += \
    core/qtllmclient.cpp \
    providers/openaicompatibleprovider.cpp \
    providers/ollamaprovider.cpp \
    providers/vllmprovider.cpp \
    network/httpexecutor.cpp \
    streaming/streamchunkparser.cpp
