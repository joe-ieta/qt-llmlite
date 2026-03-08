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
    $$PWD/streaming \
    $$PWD/chat \
    $$PWD/storage \
    $$PWD/profile \
    $$PWD/tools \
    $$PWD/tools/runtime \
    $$PWD/tools/protocol

HEADERS += \
    core/llmconfig.h \
    core/llmtypes.h \
    core/qtllmclient.h \
    providers/illmprovider.h \
    providers/openaicompatibleprovider.h \
    providers/ollamaprovider.h \
    providers/providerfactory.h \
    providers/vllmprovider.h \
    network/httpexecutor.h \
    streaming/streamchunkparser.h \
    chat/conversationsnapshot.h \
    chat/conversationclient.h \
    chat/conversationclientfactory.h \
    storage/conversationrepository.h \
    profile/clientprofile.h \
    profile/memorypolicy.h \
    tools/llmtooldefinition.h \
    tools/builtintools.h \
    tools/llmtoolregistry.h \
    tools/llmtooladapter.h \
    tools/toolselectionlayer.h \
    tools/toolenabledchatentry.h \
    tools/runtime/toolruntime_types.h \
    tools/runtime/itoolexecutor.h \
    tools/runtime/builtinexecutors.h \
    tools/runtime/toolexecutorregistry.h \
    tools/runtime/toolruntimehooks.h \
    tools/runtime/toolexecutionlayer.h \
    tools/runtime/toolcatalogrepository.h \
    tools/runtime/clienttoolpolicyrepository.h \
    tools/runtime/toolcallorchestrator.h \
    tools/protocol/itoolcallprotocoladapter.h \
    tools/protocol/providerprotocoladapters.h \
    tools/protocol/toolcallprotocolrouter.h

SOURCES += \
    core/qtllmclient.cpp \
    providers/openaicompatibleprovider.cpp \
    providers/ollamaprovider.cpp \
    providers/providerfactory.cpp \
    providers/vllmprovider.cpp \
    network/httpexecutor.cpp \
    streaming/streamchunkparser.cpp \
    chat/conversationclient.cpp \
    chat/conversationclientfactory.cpp \
    storage/conversationrepository.cpp \
    tools/llmtoolregistry.cpp \
    tools/builtintools.cpp \
    tools/llmtooladapter.cpp \
    tools/toolselectionlayer.cpp \
    tools/toolenabledchatentry.cpp \
    tools/runtime/toolexecutorregistry.cpp \
    tools/runtime/builtinexecutors.cpp \
    tools/runtime/toolexecutionlayer.cpp \
    tools/runtime/toolcatalogrepository.cpp \
    tools/runtime/clienttoolpolicyrepository.cpp \
    tools/runtime/toolcallorchestrator.cpp \
    tools/protocol/providerprotocoladapters.cpp \
    tools/protocol/toolcallprotocolrouter.cpp
