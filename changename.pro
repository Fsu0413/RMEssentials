#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = changename
TEMPLATE = app


SOURCES += main.cpp\
        ChangeNameDialog.cpp \
    utils.cpp \
    renamer.cpp \
    DownloadDialog.cpp \
    downloader.cpp

HEADERS  += ChangeNameDialog.h \
    utils.h \
    renamer.h \
    DownloadDialog.h \
    downloader.h

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    lang.qrc

TRANSLATIONS += changename.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
