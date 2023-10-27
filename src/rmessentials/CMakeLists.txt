
set(RMESSENTIALSAPP_HEADERS
    src/changename.h
    src/download.h
    src/main.h
    src/songclientedit.h
    src/papasongclientedit.h
)

set(RMESSENTIALSAPP_SOURCES
    src/main.cpp
    src/changename.cpp
    src/download.cpp
    src/songclientedit.cpp
    src/papasongclientedit.cpp
)

add_custom_target(RMEssentials ALL)
if (ANDROID)
    set(RMESSENTIALS_MOBILITY true)
endif()

if(Qt6_FOUND)
    qt6_add_executable(RMEssentials6 WIN32 MACOSX_BUNDLE ${RMESSENTIALSAPP_HEADERS} ${RMESSENTIALSAPP_SOURCES})
    add_dependencies(RMEssentials6 RmEss)
    add_dependencies(RMEssentials RMEssentials6)

    set_target_properties(RMEssentials6 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMEssentials_VERSION}"
    )

    if (ANDROID)
        set_target_properties(RMEssentials6 PROPERTIES
            QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/android"
        )
    endif()

    target_link_libraries(RMEssentials6
        PUBLIC RmEss6
        PUBLIC Qt6::Core
        PUBLIC Qt6::Gui
        PUBLIC Qt6::Widgets
        PUBLIC Qt6::Network
    )

    qt6_add_translations(RMEssentials6 TS_FILES translations/rmessentials.ts
                        LUPDATE_OPTIONS -no-obsolete
    )
    rmessentials_win32_rc(RMEssentials6 true "")

    install(TARGETS RMEssentials6
        RUNTIME
    )
endif()

if(Qt5_FOUND)
    set(RMESSENTIALSAPP_QMFILES5)
    if (NOT Qt6_FOUND)
        # Generated file path is same as Qt 6 ones. Since Qt 6 generated file is compatible with Qt 5 so we ignore Qt 5 generated ones
        qt5_add_translation(RMESSENTIALSAPP_QMFILES5 translations/rmessentials.ts)
    endif()

    if(ANDROID)
        # How to build it?
        add_library(RMEssentials5 MODULE ${RMESSENTIALSAPP_HEADERS} ${RMESSENTIALSAPP_SOURCES} ${RMESSENTIALSAPP_QMFILES5})
    else()
        add_executable(RMEssentials5 WIN32 MACOSX_BUNDLE ${RMESSENTIALSAPP_HEADERS} ${RMESSENTIALSAPP_SOURCES} ${RMESSENTIALSAPP_QMFILES5})
    endif()
    add_dependencies(RMEssentials5 RmEss)
    add_dependencies(RMEssentials RMEssentials5)

    set_target_properties(RMEssentials5 PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        VERSION "${RMEssentials_VERSION}"
    )

    if (ANDROID)
        set_target_properties(RMEssentials5 PROPERTIES
            QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/android5"
        )
    endif()

    target_link_libraries(RMEssentials5
        PUBLIC RmEss5
        PUBLIC Qt5::Core
        PUBLIC Qt5::Gui
        PUBLIC Qt5::Widgets
        PUBLIC Qt5::Network
    )

    if (WIN32)
        target_link_libraries(RMEssentials5
            PUBLIC Qt5::WinExtras
        )
    endif()
    if (ANDROID)
        target_link_libraries(RMEssentials5
            PUBLIC Qt5::AndroidExtras
        )
    endif()
    rmessentials_win32_rc(RMEssentials5 true "")

    install(TARGETS RMEssentials5
        RUNTIME
    )
endif()
