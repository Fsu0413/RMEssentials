#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = RMEssentials
TEMPLATE = app


SOURCES += ../src/main.cpp\
        ../src/ChangeNameDialog.cpp \
    ../src/utils.cpp \
    ../src/renamer.cpp \
    ../src/DownloadDialog.cpp \
    ../src/downloader.cpp \
    ../src/maindialog.cpp

HEADERS  += ../src/ChangeNameDialog.h \
    ../src/utils.h \
    ../src/renamer.h \
    ../src/DownloadDialog.h \
    ../src/downloader.h \
    ../src/maindialog.h

CONFIG += mobility
MOBILITY = 

RESOURCES += \
    ../lang.qrc

TRANSLATIONS += ../changename.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

FORMS +=
