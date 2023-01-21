# DO NOT INCLUDE $$_PRO_FILE_PWD_/../../RMEssentials.pri here

CONFIG += warn_on c++17
win32: CONFIG += skip_target_version_ext

QT = core

qtHaveModule("zlib-private") {
    QT += zlib-private
} else {
    LIBS += -lz
}

equals(QT_MAJOR_VERSION, 6): QT += core5compat

TEMPLATE = lib
CONFIG += staticlib

TARGET = quazip

DEFINES += QUAZIP_STATIC

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

SOURCES += \
    src/JlCompress.cpp \
    src/qioapi.cpp \
    src/quaadler32.cpp \
    src/quacrc32.cpp \
    src/quagzipfile.cpp \
    src/quaziodevice.cpp \
    src/quazip.cpp \
    src/quazipdir.cpp \
    src/quazipfile.cpp \
    src/quazipfileinfo.cpp \
    src/quazipnewinfo.cpp \
    src/unzip.c \
    src/zip.c \
    src/quachecksum32.cpp

HEADERS += \
    src/minizip_crypt.h \
    src/ioapi.h \
    src/JlCompress.h \
    src/quaadler32.h \
    src/quachecksum32.h \
    src/quacrc32.h \
    src/quagzipfile.h \
    src/quaziodevice.h \
    src/quazip.h \
    src/quazip_global.h \
    src/quazipdir.h \
    src/quazipfile.h \
    src/quazipfileinfo.h \
    src/quazipnewinfo.h \
    src/unzip.h \
    src/zip.h \
    src/quazip_qt_compat.h

generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/quazip/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/quazip)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/quazip) -f $$system_path($$PWD/src/)
else: generateHeaders.commands = $$PWD/../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/quazip -f $$PWD/src/

HEADERS_ABSOLUTE =
for (header, HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target
