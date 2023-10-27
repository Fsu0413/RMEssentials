
set(QUAZIP_HEADERS
    quazip/quazip/minizip_crypt.h
    quazip/quazip/ioapi.h
    quazip/quazip/JlCompress.h
    quazip/quazip/quaadler32.h
    quazip/quazip/quachecksum32.h
    quazip/quazip/quacrc32.h
    quazip/quazip/quaziodevice.h
    quazip/quazip/quazip.h
    quazip/quazip/quazip_global.h
    quazip/quazip/quazipdir.h
    quazip/quazip/quazipfile.h
    quazip/quazip/quazipfileinfo.h
    quazip/quazip/quazipnewinfo.h
    quazip/quazip/unzip.h
    quazip/quazip/zip.h
    quazip/quazip/quazip_qt_compat.h
)

set(QUAZIP_SOURCES
    quazip/quazip/JlCompress.cpp
    quazip/quazip/qioapi.cpp
    quazip/quazip/quaadler32.cpp
    quazip/quazip/quacrc32.cpp
    quazip/quazip/quaziodevice.cpp
    quazip/quazip/quazip.cpp
    quazip/quazip/quazipdir.cpp
    quazip/quazip/quazipfile.cpp
    quazip/quazip/quazipfileinfo.cpp
    quazip/quazip/quazipnewinfo.cpp
    quazip/quazip/unzip.c
    quazip/quazip/zip.c
    quazip/quazip/quachecksum32.cpp
)

add_custom_target(quazip)
rmessentials_auto_generate_header_file_for_target(quazip "${QUAZIP_HEADERS}")

if (Qt6_FOUND)
    qt6_add_library(quazip6 ${QUAZIP_HEADERS} ${QUAZIP_SOURCES})
    add_dependencies(quazip quazip6)

    set_target_properties(quazip6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMESSENTIALS_QUAZIP_VERSION}"
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(quazip6
            PRIVATE -DQUAZIP_BUILD
        )
    else()
        target_compile_definitions(quazip6
            PUBLIC -DQUAZIP_STATIC
        )
    endif()

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

    install(TARGETS quazip6
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()
if (Qt5_FOUND)
    add_library(quazip5 ${QUAZIP_HEADERS} ${QUAZIP_SOURCES})
    add_dependencies(quazip quazip5)

    set_target_properties(quazip5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMESSENTIALS_QUAZIP_VERSION}"
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(quazip5
            PRIVATE -DQUAZIP_BUILD
        )
    else()
        target_compile_definitions(quazip5
            PUBLIC -DQUAZIP_STATIC
        )
    endif()

    if (Qt5Zlib_FOUND)
        target_link_libraries(quazip5 PUBLIC Qt5::Zlib)
    else()
        target_link_libraries(quazip5 PUBLIC -lz)
    endif()

    target_link_libraries(quazip5
        PUBLIC Qt5::Core
    )
    target_include_directories(quazip5 INTERFACE ${CMAKE_BINARY_DIR}/dist/include)

    install(TARGETS quazip5
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()
