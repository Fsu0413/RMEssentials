#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network

VERSION = 6.12.24
RMEVERSION = 20161224
DEFINES += "RMEVERSION=\\\"$$RMEVERSION\\\""

win32 {
    QT += winextras
}

TARGET = RMEssentials
TEMPLATE = app

CONFIG += precompiled_header c++11 warn_on
PRECOMPILED_HEADER = src/pch.h

android {
    CONFIG += mobility
    MOBILITY =
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
} else {
    CONFIG += quazip
}

android || ios {
    DEFINES += MOBILE_DEVICES
}

TRANSLATIONS += changename.ts


win32 {
    RC_ICONS = $$_PRO_FILE_PWD_/res/1.ico
}

mac{
    ICON = res/1.icns
}

HEADERS += \
    src/core/downloader.h \
    src/core/renamer.h \
    src/core/songstruct.h \
    src/core/utils.h \
    src/dialog/ChangeNameDialog.h \
    src/dialog/DownloadDialog.h \
    src/dialog/maindialog.h \
    src/dialog/SongClientEditDialog.h \
    src/dialog/PapaSongClientEditDialog.h \
    src/pch.h \
    src/core/songfile.h

SOURCES += \
    src/core/downloader.cpp \
    src/core/main.cpp \
    src/core/renamer.cpp \
    src/core/songstruct.cpp \
    src/core/utils.cpp \
    src/dialog/ChangeNameDialog.cpp \
    src/dialog/DownloadDialog.cpp \
    src/dialog/maindialog.cpp \
    src/dialog/SongClientEditDialog.cpp \
    src/dialog/PapaSongClientEditDialog.cpp \
    src/core/songfile.cpp

INCLUDEPATH += src/core
INCLUDEPATH += src/dialog


CONFIG(quazip) {
    DEFINES += "RME_USE_QUAZIP=\\\"0.7.2\\\"" QUAZIP_STATIC
    INCLUDEPATH += src/quazip

    SOURCES += \
        src/quazip/JlCompress.cpp \
        src/quazip/qioapi.cpp \
        src/quazip/quaadler32.cpp \
        src/quazip/quacrc32.cpp \
        src/quazip/quagzipfile.cpp \
        src/quazip/quaziodevice.cpp \
        src/quazip/quazip.cpp \
        src/quazip/quazipdir.cpp \
        src/quazip/quazipfile.cpp \
        src/quazip/quazipfileinfo.cpp \
        src/quazip/quazipnewinfo.cpp \
        src/quazip/unzip.c \
        src/quazip/zip.c \
        src/core/uncompresser.cpp

    HEADERS += \
        src/quazip/crypt.h \
        src/quazip/ioapi.h \
        src/quazip/JlCompress.h \
        src/quazip/quaadler32.h \
        src/quazip/quachecksum32.h \
        src/quazip/quacrc32.h \
        src/quazip/quagzipfile.h \
        src/quazip/quaziodevice.h \
        src/quazip/quazip.h \
        src/quazip/quazip_global.h \
        src/quazip/quazipdir.h \
        src/quazip/quazipfile.h \
        src/quazip/quazipfileinfo.h \
        src/quazip/quazipnewinfo.h \
        src/quazip/unzip.h \
        src/quazip/zip.h \
        src/core/uncompresser.h \

    qtHaveModule("zlib-private") {
        QT += zlib-private
    } else {
        LIBS += -lz
    }
}

LIBS += -L.

linux{
    !android {
        !contains(QMAKE_HOST.arch, x86_64) {
            QMAKE_LFLAGS += -Wl,--rpath=lib/linux/x86
        } else {
            QMAKE_LFLAGS += -Wl,--rpath=lib/linux/x64
        }
    }
}
