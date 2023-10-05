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

equals(QT_MAJOR_VERSION, 6): TARGET = quazip
else: TARGET = quazip$$qtPlatformTargetSuffix()

staticlib: DEFINES += QUAZIP_STATIC
else: DEFINES += QUAZIP_BUILD

DESTDIR = $$OUT_PWD/../dist/lib
DLLDESTDIR = $$OUT_PWD/../dist/bin

# quagzipfile.cpp / h omitted. Is of no use.

SOURCES += \
    quazip/quazip/JlCompress.cpp \
    quazip/quazip/qioapi.cpp \
    quazip/quazip/quaadler32.cpp \
    quazip/quazip/quacrc32.cpp \
    quazip/quazip/quaziodevice.cpp \
    quazip/quazip/quazip.cpp \
    quazip/quazip/quazipdir.cpp \
    quazip/quazip/quazipfile.cpp \
    quazip/quazip/quazipfileinfo.cpp \
    quazip/quazip/quazipnewinfo.cpp \
    quazip/quazip/unzip.c \
    quazip/quazip/zip.c \
    quazip/quazip/quachecksum32.cpp

HEADERS += \
    quazip/quazip/minizip_crypt.h \
    quazip/quazip/ioapi.h \
    quazip/quazip/JlCompress.h \
    quazip/quazip/quaadler32.h \
    quazip/quazip/quachecksum32.h \
    quazip/quazip/quacrc32.h \
    quazip/quazip/quaziodevice.h \
    quazip/quazip/quazip.h \
    quazip/quazip/quazip_global.h \
    quazip/quazip/quazipdir.h \
    quazip/quazip/quazipfile.h \
    quazip/quazip/quazipfileinfo.h \
    quazip/quazip/quazipnewinfo.h \
    quazip/quazip/unzip.h \
    quazip/quazip/zip.h \
    quazip/quazip/quazip_qt_compat.h

generateHeaders.target = $$system_path($$OUT_PWD/../dist/include/quazip/.timestamp)
!build_pass: mkpath($$OUT_PWD/../dist/include/quazip)

contains(QMAKE_HOST.os, "Windows"): generateHeaders.commands = cscript $$system_path($$PWD/../../tools/AutoGenerateHeader.vbs) -o $$system_path($$OUT_PWD/../dist/include/quazip) -f $$system_path($$PWD/quazip/quazip/)
else: generateHeaders.commands = $$PWD/../../tools/AutoGenerateHeader.sh -o $$OUT_PWD/../dist/include/quazip -f $$PWD/quazip/quazip/

HEADERS_ABSOLUTE =
for (header, HEADERS): HEADERS_ABSOLUTE += $$system_path($$absolute_path($$header))

generateHeaders.depends = $$HEADERS_ABSOLUTE

QMAKE_EXTRA_TARGETS += generateHeaders
PRE_TARGETDEPS += $$generateHeaders.target
