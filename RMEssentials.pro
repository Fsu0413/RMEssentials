#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = RMEssentials
TEMPLATE = app


SOURCES += src/main.cpp\
        src/ChangeNameDialog.cpp \
    src/utils.cpp \
    src/renamer.cpp \
    src/DownloadDialog.cpp \
    src/downloader.cpp \
    src/maindialog.cpp \
    src/uncompresser.cpp \
    src/SongClientChangeDialog.cpp \
    src/songstruct.cpp

HEADERS  += src/ChangeNameDialog.h \
    src/utils.h \
    src/renamer.h \
    src/DownloadDialog.h \
    src/downloader.h \
    src/maindialog.h \
    src/uncompresser.h \
    src/SongClientChangeDialog.h \
    src/songstruct.h

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    lang.qrc

TRANSLATIONS += changename.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

INCLUDEPATH += include/quazip

LIBS += -L.
win32-msvc*{
    DEFINES += _CRT_SECURE_NO_WARNINGS
    !contains(QMAKE_HOST.arch, x86_64) {
        DEFINES += WIN32
        LIBS += -L"$$_PRO_FILE_PWD_/lib/win32"
    } else {
        DEFINES += WIN64
        LIBS += -L"$$_PRO_FILE_PWD_/lib/win64"
    }
}
win32-g++{
    DEFINES += WIN32
    LIBS += -L"$$_PRO_FILE_PWD_/lib/MinGW"
    DEFINES += GPP
}
winrt{
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += WINRT
    LIBS += -L"$$_PRO_FILE_PWD_/lib/winrt/x64"
}
macx{
    DEFINES += MAC
    LIBS += -L"$$_PRO_FILE_PWD_/lib/mac/lib"
}
ios{
    DEFINES += IOS
    CONFIG(iphonesimulator){
        LIBS += -L"$$_PRO_FILE_PWD_/lib/ios/simulator/lib"
    }
    else {
        LIBS += -L"$$_PRO_FILE_PWD_/lib/ios/device/lib"
    }
}
linux{
    android{
        DEFINES += ANDROID
        ANDROID_LIBPATH = $$_PRO_FILE_PWD_/lib/android/$$ANDROID_ARCHITECTURE/lib
        LIBS += -L"$$ANDROID_LIBPATH"
    }
    else {
        DEFINES += LINUX
        !contains(QMAKE_HOST.arch, x86_64) {
            LIBS += -L"$$_PRO_FILE_PWD_/lib/linux86"
        }
        else {
            LIBS += -L"$$_PRO_FILE_PWD_/lib/linux64"
        }
    }
}

LIBS += -lquazip
DEFINES += QUAZIP_STATIC
