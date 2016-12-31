
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

generateHeaders.target = ../dist/include/RMEssentials
contains(QMAKE_HOST.os, "Windows") {
    mkdirGenerateHeaders.commands = if not exist $$system_path($$generateHeaders.target) md $$system_path($$generateHeaders.target)
    generateHeaders.commands = cscript $$system_path($$PWD/../../tools/AutoGenerateHeader.vbs) -o $$system_path($$generateHeaders.target) -f $$system_path($$PWD/src/)
} else {
    mkdirGenerateHeaders.commands = mkdir -p $$generateHeaders.target
    generateHeaders.commands = $$PWD/../../tools/AutoGenerateHeader.sh -o $$generateHeaders.target -f $$PWD/src/
}
generateHeaders.depends = mkdirGenerateHeaders

QMAKE_EXTRA_TARGETS += mkdirGenerateHeaders generateHeaders
POST_TARGETDEPS += $$generateHeaders.target
