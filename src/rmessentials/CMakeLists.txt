
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

    set_property(TARGET RMEssentials6 PROPERTY AUTOMOC ON)
    set_property(TARGET RMEssentials6 PROPERTY AUTOUIC ON)
    set_property(TARGET RMEssentials6 PROPERTY AUTORCC ON)

    if (ANDROID)
        set_property(TARGET RMEssentials6 QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/android)
    endif()

    target_link_libraries(RMEssentials6
        PUBLIC RmEss6
        PUBLIC Qt6::Core
        PUBLIC Qt6::Gui
        PUBLIC Qt6::Widgets
        PUBLIC Qt6::Network
    )
endif()

if(Qt5_FOUND)
    if(ANDROID)
        # How to build it?
        add_library(RMEssentials5 MODULE ${RMESSENTIALSAPP_HEADERS} ${RMESSENTIALSAPP_SOURCES})
        set_property(TARGET RMEssentials5 QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/android5)
    else()
        add_executable(RMEssentials5 WIN32 MACOSX_BUNDLE ${RMESSENTIALSAPP_HEADERS} ${RMESSENTIALSAPP_SOURCES})
    endif()
    add_dependencies(RMEssentials5 RmEss)
    add_dependencies(RMEssentials RMEssentials5)

    set_property(TARGET RMEssentials5 PROPERTY AUTOMOC ON)
    set_property(TARGET RMEssentials5 PROPERTY AUTOUIC ON)
    set_property(TARGET RMEssentials5 PROPERTY AUTORCC ON)

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
endif()
