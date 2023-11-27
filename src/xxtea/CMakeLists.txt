
set(XXTEA_HEADERS
    xxtea-c/xxtea.h
)

set(XXTEA_SOURCES
    xxtea-c/xxtea.c
)

add_library(xxtea EXCLUDE_FROM_ALL ${XXTEA_HEADERS} ${XXTEA_SOURCES})
rmessentials_auto_generate_header_file_for_target(xxtea "${XXTEA_HEADERS}")
set_target_properties(xxtea PROPERTIES
    VERSION 1.0.1
    WINDOWS_EXPORT_ALL_SYMBOLS true
)

if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(xxtea INTERFACE RMESSENTIALS_BUILD_STATIC_XXTEA)
endif()

target_include_directories(xxtea INTERFACE ${CMAKE_BINARY_DIR}/build/include)

install(TARGETS xxtea
    RUNTIME
    ARCHIVE
    LIBRARY
)
