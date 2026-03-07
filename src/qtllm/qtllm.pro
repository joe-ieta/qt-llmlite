TEMPLATE = lib
TARGET = qtllm
CONFIG += c++17 warn_on staticlib
QT += core network

DEFINES += QTLLM_LIBRARY

# Keep library output predictable across platforms and shadow builds.
QTLLM_LIB_DIR = $$OUT_PWD/lib
CONFIG(debug, debug|release): QTLLM_LIB_DIR = $$OUT_PWD/lib/debug
CONFIG(release, debug|release): QTLLM_LIB_DIR = $$OUT_PWD/lib/release
DESTDIR = $$QTLLM_LIB_DIR

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