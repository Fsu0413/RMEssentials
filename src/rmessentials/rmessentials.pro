
include($$_PRO_FILE_PWD_/../../RMEssentials.pri)

TEMPLATE = app
TARGET = RMEssentials

QT += core gui widgets network
win32 {
    equals(QT_MAJOR_VERSION, 5): QT += winextras
}

android {
    CONFIG += mobility
    MOBILITY =
    equals(QT_MAJOR_VERSION, 6): ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    else: ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android5

    equals(QT_MAJOR_VERSION, 6): QT += core-private
    else: QT += androidextras
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
    src/papasongclientedit.h

SOURCES += \
    src/main.cpp \
    src/changename.cpp \
    src/download.cpp \
    src/songclientedit.cpp \
    src/papasongclientedit.cpp

LIBS += -L. -lRMEss$$qtPlatformTargetSuffix()

staticlib: use_quazip {
    equals(QT_MAJOR_VERSION, 6): QT += core5compat
    LIBS += -lquazip$$qtPlatformTargetSuffix()
}

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
