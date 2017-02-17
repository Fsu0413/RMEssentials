
include($$_PRO_FILE_PWD_/../../plugins.pri)

qtHaveModule("zlib-private") {
    QT += zlib-private
} else {
    LIBS += -lz
}

DEFINES += QUAZIP_STATIC
DEFINES -= QT_NO_CAST_FROM_ASCII
*-g++: QMAKE_CXXFLAGS -= -Wzero-as-null-pointer-constant

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
    quazipplugin.cpp

HEADERS += \
    src/crypt.h \
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
    quazipplugin.h

INCLUDEPATH += src

TARGET = rmeuncomplibquazip
