TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src/qtllm \
    tests/qtllm_tests \
    src/apps/simple_chat \
    src/apps/multi_client_chat \
    src/apps/mcp_server_manager \
    src/apps/tools_inside \
    src/apps/toolstudio

tests/qtllm_tests.depends = src/qtllm
src/apps/simple_chat.depends = src/qtllm
src/apps/multi_client_chat.depends = src/qtllm
src/apps/mcp_server_manager.depends = src/qtllm

src/apps/tools_inside.depends = src/qtllm
src/apps/toolstudio.depends = src/qtllm
