
include($$_PRO_FILE_PWD_/../../RMEssentials.pri)

TEMPLATE = app
TARGET = RMEssentials

QT += core gui widgets network
win32 {
    QT += winextras
}

android {
    CONFIG += mobility
    MOBILITY =
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
} else {
    CONFIG += quazip
}

TRANSLATIONS += translations/rmessentials.ts
DISTFILES += $$TRANSLATIONS \
    android/AndroidManifest.xml \
    android/res/values/libs.xml \
    android/build.gradle

win32 {
    RC_ICONS = res/1.ico
}

osx {
    ICON = res/1.icns
}

HEADERS += \
    src/changename.h \
    src/download.h \
    src/main.h \
    src/songclientedit.h \
    src/papasongclientedit.h \
    src/pastelineedit.h

SOURCES += \
    src/main.cpp \
    src/changename.cpp \
    src/download.cpp \
    src/songclientedit.cpp \
    src/papasongclientedit.cpp \
    src/pastelineedit.cpp

LIBS += -L. -lRMEss

unix: !osx: !ios: !android: !install_build {
    !contains(QMAKE_HOST.arch, x86_64) {
        QMAKE_LFLAGS += -Wl,--rpath=lib/x86
    } else {
        QMAKE_LFLAGS += -Wl,--rpath=lib/x64
    }
}

DESTDIR = $$OUT_PWD/../dist/bin

target.path = /bin/
INSTALLS += target
