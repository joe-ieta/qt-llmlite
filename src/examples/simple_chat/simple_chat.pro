TEMPLATE = app
TARGET = simple_chat
CONFIG += c++17 warn_on
QT += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
    ../../qtllm \
    ../../qtllm/core \
    ../../qtllm/providers \
    ../../qtllm/network \
    ../../qtllm/streaming

HEADERS += \
    chatwindow.h

SOURCES += \
    main.cpp \
    chatwindow.cpp

LIBS += -L../../qtllm -lqtllm

PRE_TARGETDEPS += ../../qtllm/libqtllm.a
