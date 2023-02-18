
set(LIBRMESS_HEADERS
    src/rmedownloader.h
    src/rmeglobal.h
    src/rmerenamer.h
    src/rmesongfile.h
    src/rmesongstruct.h
    src/rmeuncompresser.h
    src/rmeutils.h
)

set(LIBRMESS_SOURCES
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

add_custom_target(RmEss)
set(LIBRMESSENTIALS_JPG2PNGHACK true)

if (Qt6_FOUND)
    qt6_add_library(RmEss6 ${LIBRMESS_HEADERS} ${LIBRMESS_SOURCES} ${LIBRMESS_RESOURCES})
    add_dependencies(RmEss RmEss6)

    set_property(TARGET RmEss6 PROPERTY AUTOMOC ON)
    set_property(TARGET RmEss6 PROPERTY AUTOUIC ON)
    set_property(TARGET RmEss6 PROPERTY AUTORCC ON)

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
    )

    if (RMESSENTIALS_USE_QUAZIP)
        add_dependencies(RmEss6 quazip)
        target_link_libraries(RmEss6 PUBLIC quazip6)

        target_compile_definitions(RmEss6
            PUBLIC -DRME_USE_QUAZIP="${RMESSENTIALS_QUAZIP_VERSION}"
        )
    endif()
    target_link_libraries(RmEss6
        PUBLIC Qt6::Core
        PUBLIC Qt6::Network
    )
    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_link_libraries(RmEss6
            PUBLIC Qt6::Gui
        )
    endif()
    target_include_directories(RmEss6 PUBLIC ${CMAKE_BINARY_DIR}/dist/include)
endif()

if (Qt5_FOUND)
    add_library(RmEss5 ${LIBRMESS_HEADERS} ${LIBRMESS_SOURCES} ${LIBRMESS_RESOURCES})
    add_dependencies(RmEss RmEss5)

    set_property(TARGET RmEss5 PROPERTY AUTOMOC ON)
    set_property(TARGET RmEss5 PROPERTY AUTOUIC ON)
    set_property(TARGET RmEss5 PROPERTY AUTORCC ON)

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
    )

    if (RMESSENTIALS_USE_QUAZIP)
        add_dependencies(RmEss5 quazip)
        target_link_libraries(RmEss5 PUBLIC quazip5)
        target_compile_definitions(RmEss5
            PUBLIC -DRME_USE_QUAZIP="${RMESSENTIALS_QUAZIP_VERSION}"
        )
    endif()
    target_link_libraries(RmEss5
        PUBLIC Qt5::Core
        PUBLIC Qt5::Network
    )
    if (LIBRMESSENTIALS_JPG2PNGHACK)
        target_link_libraries(RmEss5
            PUBLIC Qt5::Gui
        )
    endif()
    target_include_directories(RmEss5 PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
endif()

if(CMAKE_HOST_WIN32)
    set(RMESSENTIALS_RMESS_GENERATE_HEADERS_EXECUTABLE cscript ${CMAKE_SOURCE_DIR}/tools/AutoGenerateHeader.vbs)
else()
    set(RMESSENTIALS_RMESS_GENERATE_HEADERS_EXECUTABLE ${CMAKE_SOURCE_DIR}/tools/AutoGenerateHeader.sh)
endif()
add_custom_command(TARGET RmEss
    PRE_BUILD
    COMMAND ${RMESSENTIALS_RMESS_GENERATE_HEADERS_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/dist/include/RmEss -f ${CMAKE_CURRENT_LIST_DIR}/src
    # BYPRODUCTS ${CMAKE_BINARY_DIR}/dist/include/quazip
)
