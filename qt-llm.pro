TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    src/qtllm \
    tests/qtllm_tests \
    src/examples/simple_chat \
    src/examples/multi_client_chat \
    src/examples/mcp_server_manager_demo \
    src/examples/tools_inside

tests/qtllm_tests.depends = src/qtllm
src/examples/simple_chat.depends = src/qtllm
src/examples/multi_client_chat.depends = src/qtllm
src/examples/mcp_server_manager_demo.depends = src/qtllm

src/examples/tools_inside.depends = src/qtllm
