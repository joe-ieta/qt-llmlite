TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src/qtllm \
    src/examples/simple_chat

src/examples/simple_chat.depends = src/qtllm
