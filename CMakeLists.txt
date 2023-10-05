cmake_minimum_required(VERSION 3.19)

project(RMEssentials
    VERSION 13.2.18
    DESCRIPTION "Program to manipulating RMEssentials files"
    HOMEPAGE_URL "https://github.com/Fsu0413/RMEssentials"
    LANGUAGES C CXX
)

set(RMEVERSION 20230218)

set(CMAKE_CXX_STANDARD 17)

if(ANDROID OR IOS)
    set(RMESSENTIALS_MOBILE_DEVICES true)
else()
    set(RMESSENTIALS_MOBILE_DEVICES false)
endif()

if (RMESSENTIALS_USE_QUAZIP)
    set(RMESSENTIALS_FORCE_USE_QUAZIP true)
endif()

option(RMESSENTIALS_USE_QUAZIP "use QuaZip" true)
option(BUILD_SHARED_LIBS "Build Shared Libraries" true)

set(CMAKE_VERBOSE_MAKEFILE true)

if(NOT MSVC)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -Wpointer-to-int-cast)
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wc++17-compat)
    #    *-g++: QMAKE_CXXFLAGS += -Wzero-as-null-pointer-constant
    #    mac:QMAKE_LFLAGS += -Wl,-undefined -Wl,error
    #    else:QMAKE_LFLAGS += -Wl,--no-undefined
endif()

set(RMESSENTIALS_QT6_COMPONENTS Core Gui Widgets Network LinguistTools)
set(RMESSENTIALS_QT5_COMPONENTS Core Gui Widgets Network LinguistTools)
if (WIN32)
    set(RMESSENTIALS_QT5_COMPONENTS ${RMESSENTIALS_QT5_COMPONENTS} WinExtras)
elseif (ANDROID)
    set(RMESSENTIALS_QT5_COMPONENTS ${RMESSENTIALS_QT5_COMPONENTS} AndroidExtras)
endif()
find_package(Qt6 COMPONENTS ${RMESSENTIALS_QT6_COMPONENTS} OPTIONAL_COMPONENTS ZlibPrivate Core5Compat)
find_package(Qt5 5.15 COMPONENTS ${RMESSENTIALS_QT5_COMPONENTS} OPTIONAL_COMPONENTS Zlib)

if (NOT (Qt5_FOUND OR Qt6_FOUND))
    message(FATAL_ERROR "at least one of Qt 5 and Qt 6 is needed")
endif()

if (Qt6_FOUND)
    message("Using Qt6")
    if (NOT Qt6Core5Compat_FOUND)
        set(RMESSENTIALS_USE_QUAZIP false)
        if (RMESSENTIALS_FORCE_USE_QUAZIP)
            message(FATAL_ERROR "QuaZip can't be used without Core5Compat on Qt6")
        endif()
    else()
    endif()
endif()
if (Qt5_FOUND)
    message("Using Qt5")
endif()

if(RMESSENTIALS_USE_QUAZIP)
    set(RMESSENTIALS_QUAZIP_VERSION "1.4")
endif()

add_subdirectory(src)
