#-----------------------------------------------------------------------------
#
# examples
#
#-----------------------------------------------------------------------------
ADD_EXECUTABLE(example_http_get example_http_get.c)
SET_TARGET_PROPERTIES(example_http_get PROPERTIES LINKER_LANGUAGE C)
TARGET_LINK_LIBRARIES(example_http_get
        json_static
        http_client_static
        netty_static
        byte_buffer_static
        tiny_string_util_static
        tiny_container_static
        tiny_log_static
        tiny_portable_static
        tiny_base_static
        ${OS_LIB})

ADD_EXECUTABLE(example_http_post example_http_post.c)
SET_TARGET_PROPERTIES(example_http_post PROPERTIES LINKER_LANGUAGE C)
TARGET_LINK_LIBRARIES(example_http_post
        json_static
        http_client_static
        netty_static
        byte_buffer_static
        tiny_string_util_static
        tiny_container_static
        tiny_log_static
        tiny_portable_static
        tiny_base_static
        ${OS_LIB})