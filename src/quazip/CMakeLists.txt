
set(QUAZIP_HEADERS
    src/minizip_crypt.h
    src/ioapi.h
    src/JlCompress.h
    src/quaadler32.h
    src/quachecksum32.h
    src/quacrc32.h
    src/quagzipfile.h
    src/quaziodevice.h
    src/quazip.h
    src/quazip_global.h
    src/quazipdir.h
    src/quazipfile.h
    src/quazipfileinfo.h
    src/quazipnewinfo.h
    src/unzip.h
    src/zip.h
    src/quazip_qt_compat.h
)

set(QUAZIP_SOURCES
    src/JlCompress.cpp
    src/qioapi.cpp
    src/quaadler32.cpp
    src/quacrc32.cpp
    src/quagzipfile.cpp
    src/quaziodevice.cpp
    src/quazip.cpp
    src/quazipdir.cpp
    src/quazipfile.cpp
    src/quazipfileinfo.cpp
    src/quazipnewinfo.cpp
    src/unzip.c
    src/zip.c
    src/quachecksum32.cpp
)

add_custom_target(quazip)

if (Qt6_FOUND)
    qt6_add_library(quazip6 STATIC ${QUAZIP_HEADERS} ${QUAZIP_SOURCES})
    add_dependencies(quazip quazip6)

    target_compile_definitions(quazip6
        PUBLIC -DQUAZIP_STATIC
    )

    set_property(TARGET quazip6 PROPERTY AUTOMOC ON)
    set_property(TARGET quazip6 PROPERTY AUTOUIC ON)
    set_property(TARGET quazip6 PROPERTY AUTORCC ON)

    if (Qt6ZlibPrivate_FOUND)
        target_link_libraries(quazip6 PUBLIC Qt6::ZlibPrivate)
    else()
        target_link_libraries(quazip6 PUBLIC -lz)
    endif()

    target_link_libraries(quazip6
        PUBLIC Qt6::Core
        PUBLIC Qt6::Core5Compat
    )
    target_include_directories(quazip6 INTERFACE ${CMAKE_BINARY_DIR}/dist/include)
endif()
if (Qt5_FOUND)
    add_library(quazip5 STATIC ${QUAZIP_HEADERS} ${QUAZIP_SOURCES})
    add_dependencies(quazip quazip5)

    target_compile_definitions(quazip5
        PUBLIC -DQUAZIP_STATIC
    )

    set_property(TARGET quazip5 PROPERTY AUTOMOC ON)
    set_property(TARGET quazip5 PROPERTY AUTOUIC ON)
    set_property(TARGET quazip5 PROPERTY AUTORCC ON)

    if (Qt5Zlib_FOUND)
        target_link_libraries(quazip5 PUBLIC Qt5::Zlib)
    else()
        target_link_libraries(quazip5 PUBLIC -lz)
    endif()

    target_link_libraries(quazip5
        PUBLIC Qt5::Core
    )
    target_include_directories(quazip5 INTERFACE ${CMAKE_BINARY_DIR}/dist/include)
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/dist/include/quazip)

if(CMAKE_HOST_WIN32)
    set(RMESSENTIALS_QUAZIP_GENERATE_HEADERS_EXECUTABLE cscript ${CMAKE_SOURCE_DIR}/tools/AutoGenerateHeader.vbs)
else()
    set(RMESSENTIALS_QUAZIP_GENERATE_HEADERS_EXECUTABLE ${CMAKE_SOURCE_DIR}/tools/AutoGenerateHeader.sh)
endif()

add_custom_command(TARGET quazip
    PRE_BUILD
    COMMAND ${RMESSENTIALS_QUAZIP_GENERATE_HEADERS_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/dist/include/quazip -f ${CMAKE_CURRENT_LIST_DIR}/src
    # BYPRODUCTS ${CMAKE_BINARY_DIR}/dist/include/quazip
)
