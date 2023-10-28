
set(LIBRMESS_HEADERS
    src/rmechart.h
    src/rmedownloader.h
    src/rmeglobal.h
    src/rmerenamer.h
    src/rmesongfile.h
    src/rmesongstruct.h
    src/rmeuncompresser.h
    src/rmeutils.h
)

set(LIBRMESS_SOURCES
    src/rmechart.cpp
    src/rmedownloader.cpp
    src/rmerenamer.cpp
    src/rmesongfile.cpp
    src/rmesongstruct.cpp
    src/rmeuncompresser.cpp
    src/rmeutils.cpp
    src/rmeglobal.cpp
)

set(LIBRMESS_RESOURCES
    res/strtemp.qrc
)

add_custom_target(RmEss ALL)
rmessentials_auto_generate_header_file_for_target(RmEss "${LIBRMESS_HEADERS}")

set(LIBRMESSENTIALS_JPG2PNGHACK true)

if (Qt6_FOUND)
    qt6_add_library(RmEss6 ${LIBRMESS_HEADERS} ${LIBRMESS_SOURCES} ${LIBRMESS_RESOURCES})
    add_dependencies(RmEss RmEss6)

    if (BUILD_TESTING AND NOT MSVC)
        target_compile_options(RmEss6
            PRIVATE "--coverage"
        )
        target_link_options(RmEss6
            PRIVATE "--coverage"
        )
    endif()

    set_target_properties(RmEss6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMEssentials_VERSION}"
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(RmEss6 PRIVATE -DLIBRMESSENTIALS_BUILD)
    else()
        target_compile_definitions(RmEss6 PUBLIC -DLIBRMESSENTIALS_STATIC)
    endif()

    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_compile_definitions(RmEss6 PRIVATE -DLIBRMESS_JPG2PNGHACK)
    endif()

    target_compile_definitions(RmEss6
        PUBLIC -DRMEVERSION="${RMEVERSION}"
        PUBLIC -DRMEVERSIONNUMBER="${PROJECT_VERSION}"
        PUBLIC -DQT_DISABLE_DEPRECATED_BEFORE=0x70000

        PUBLIC -DQT_CC_WARNINGS
    )

    target_link_libraries(RmEss6
        PUBLIC Qt6::Core
        PUBLIC Qt6::Network
    )
    if (RMESSENTIALS_USE_QUAZIP)
        add_dependencies(RmEss6 quazip)
        target_link_libraries(RmEss6 PUBLIC quazip6)

        target_compile_definitions(RmEss6
            PUBLIC -DRME_USE_QUAZIP="${RMESSENTIALS_QUAZIP_VERSION}"
        )
    else()
        target_link_libraries(RmEss6
            PRIVATE Qt6::GuiPrivate
        )
    endif()
    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_link_libraries(RmEss6
            PUBLIC Qt6::Gui
        )
    endif()
    target_include_directories(RmEss6 PUBLIC ${CMAKE_BINARY_DIR}/dist/include)
    rmessentials_win32_rc(RmEss6 false "")

    install(TARGETS RmEss6
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()

if (Qt5_FOUND)
    add_library(RmEss5 ${LIBRMESS_HEADERS} ${LIBRMESS_SOURCES} ${LIBRMESS_RESOURCES})
    add_dependencies(RmEss RmEss5)

    if (BUILD_TESTING AND NOT MSVC)
        target_compile_options(RmEss5
            PRIVATE "--coverage"
        )
        target_link_options(RmEss5
            PRIVATE "--coverage"
        )
    endif()

    set_target_properties(RmEss5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMEssentials_VERSION}"
    )

    if (BUILD_SHARED_LIBS)
        target_compile_definitions(RmEss5 PRIVATE -DLIBRMESSENTIALS_BUILD)
    else()
        target_compile_definitions(RmEss5 PUBLIC -DLIBRMESSENTIALS_STATIC)
    endif()

    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_compile_definitions(RmEss5 PRIVATE -DLIBRMESS_JPG2PNGHACK)
    endif()
    target_compile_definitions(RmEss5
        PUBLIC -DRMEVERSION="${RMEVERSION}"
        PUBLIC -DRMEVERSIONNUMBER="${PROJECT_VERSION}"
        PUBLIC -DQT_DISABLE_DEPRECATED_BEFORE=0x70000

        PUBLIC -DQT_CC_WARNINGS
    )

    target_link_libraries(RmEss5
        PUBLIC Qt5::Core
        PUBLIC Qt5::Network
    )
    if (RMESSENTIALS_USE_QUAZIP)
        add_dependencies(RmEss5 quazip)
        target_link_libraries(RmEss5 PUBLIC quazip5)
        target_compile_definitions(RmEss5
            PUBLIC -DRME_USE_QUAZIP="${RMESSENTIALS_QUAZIP_VERSION}"
        )
    else()
        target_link_libraries(RmEss5
            PRIVATE Qt5::GuiPrivate
        )
    endif()
    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_link_libraries(RmEss5
            PUBLIC Qt5::Gui
        )
    endif()
    target_include_directories(RmEss5 PUBLIC ${CMAKE_BINARY_DIR}/dist/include)
    rmessentials_win32_rc(RmEss5 false "")

    install(TARGETS RmEss5
        RUNTIME
        ARCHIVE
        LIBRARY
    )
endif()

add_subdirectory(test)
