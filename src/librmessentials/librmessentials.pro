
include($$_PRO_FILE_PWD_/../../RMEssentials.pri)

QT += core gui network

use_quazip {
    qtHaveModule("zlib-private") {
        QT += zlib-private
    } else {
        LIBS += -lz
    }

    LIBS += -lquazip
    DEFINES += QUAZIP_STATIC
}

TEMPLATE = lib
TARGET = RMEssentials

!staticlib: DEFINES += LIBRMESSENTIALS_BUILD

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
    src/rmeutils.cpp

generateHeaders.target = $$OUT_PWD/../dist/include/RMEssentials/.timestamp
!build_pass: mkpath($$OUT_PWD/../dist/include/RMEssentials)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/RMEssentials) -f $$system_path($$PWD/src/)
else: generateHeaders.commands = $$PWD/../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/RMEssentials -f $$PWD/src/

HEADERS_ABSOLUTE =
for (header, HEADERS): HEADERS_ABSOLUTE += $$absolute_path($$header)

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target

includetarget.path = /include/RMEssentials/
includetarget.files = $$OUT_PWD/../dist/include/RMEssentials/*

target.path = /lib/
dlltarget.path = /bin/
INSTALLS += target dlltarget includetarget