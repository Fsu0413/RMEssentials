
include($$_PRO_FILE_PWD_/../../RMEssentials.pri)

QT += core network

use_quazip {
    qtHaveModule("zlib-private") {
        QT += zlib-private
    } else {
        LIBS += -lz
    }

    equals(QT_MAJOR_VERSION, 6): QT += core5compat

    LIBS += -lquazip$$qtPlatformTargetSuffix()
    DEFINES += QUAZIP_STATIC
}

TEMPLATE = lib
TARGET = RMEss

!staticlib: DEFINES += LIBRMESSENTIALS_BUILD

CONFIG += LIBRMESS_JPG2PNGHACK

CONFIG(LIBRMESS_JPG2PNGHACK) {
    QT += gui
    DEFINES += LIBRMESS_JPG2PNGHACK
}

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

HEADERS += \
    src/rmedownloader.h \
    src/rmeglobal.h \
    src/rmerenamer.h \
    src/rmesongfile.h \
    src/rmesongstruct.h \
    src/rmeuncompresser.h \
    src/rmeutils.h

SOURCES += \
    src/rmedownloader.cpp \
    src/rmerenamer.cpp \
    src/rmesongfile.cpp \
    src/rmesongstruct.cpp \
    src/rmeuncompresser.cpp \
    src/rmeutils.cpp \
    src/rmeglobal.cpp

generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/RMEss/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/RMEss)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/RMEss) -f $$system_path($$PWD/src/)
else: generateHeaders.commands = $$PWD/../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/RMEss -f $$PWD/src/

HEADERS_ABSOLUTE =
for (header, HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target

includetarget.path = /include/RMEss/
includetarget.files = $$OUT_PWD/../dist/include/RMEss/*

target.path = /lib/
dlltarget.path = /bin/
INSTALLS += target dlltarget includetarget

RESOURCES += \
    res/strtemp.qrc
