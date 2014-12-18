#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = RMEssentials
TEMPLATE = app



CONFIG += mobility
MOBILITY = 

RESOURCES += \
    lang.qrc

TRANSLATIONS += changename.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

win32{
    RC_FILE += res/icon.rc
}

mac{
    ICON = res/1.icns
}

DEFINES += QUAZIP_STATIC

HEADERS += \
    src/core/downloader.h \
    src/core/renamer.h \
    src/core/songstruct.h \
    src/core/uncompresser.h \
    src/core/utils.h \
    src/dialog/ChangeNameDialog.h \
    src/dialog/DownloadDialog.h \
    src/dialog/maindialog.h \
    src/dialog/SongClientChangeDialog.h \
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
    src/quazip/zconf.h \
    src/quazip/zip.h \
    src/quazip/zlib.h

SOURCES += \
    src/core/downloader.cpp \
    src/core/main.cpp \
    src/core/renamer.cpp \
    src/core/songstruct.cpp \
    src/core/uncompresser.cpp \
    src/core/utils.cpp \
    src/dialog/ChangeNameDialog.cpp \
    src/dialog/DownloadDialog.cpp \
    src/dialog/maindialog.cpp \
    src/dialog/SongClientChangeDialog.cpp \
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
    src/quazip/zip.c

INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/quazip

LIBS += -L.

