
include($$_PRO_FILE_PWD_/../../plugins.pri)

CONFIG += unrarlicense

HEADERS += \
    libunrarplugin.h

SOURCES += \
    libunrarplugin.cpp

TARGET = rmeuncomplibunrar

*-g++{
QMAKE_CXXFLAGS -= -Wzero-as-null-pointer-constant
}
unrarlicense {
    DEFINES += RARDLL UNRAR SILENT
    SOURCES += \
        src/archive.cpp \
        src/arcread.cpp \
        src/blake2s.cpp \
        src/cmddata.cpp \
        src/consio.cpp \
        src/crc.cpp \
        src/crypt.cpp \
        src/dll.cpp \
        src/encname.cpp \
        src/errhnd.cpp \
        src/extinfo.cpp \
        src/extract.cpp \
        src/filcreat.cpp \
        src/file.cpp \
        src/filefn.cpp \
        src/filestr.cpp \
        src/find.cpp \
        src/getbits.cpp \
        src/global.cpp \
        src/hash.cpp \
        src/headers.cpp \
        src/isnt.cpp \
        src/match.cpp \
        src/options.cpp \
        src/pathfn.cpp \
        src/qopen.cpp \
        src/rar.cpp \
        src/rarpch.cpp \
        src/rarvm.cpp \
        src/rawread.cpp \
        src/rdwrfn.cpp \
        src/rijndael.cpp \
        src/rs.cpp \
        src/rs16.cpp \
        src/scantree.cpp \
        src/secpassword.cpp \
        src/sha1.cpp \
        src/sha256.cpp \
        src/smallfn.cpp \
        src/strfn.cpp \
        src/strlist.cpp \
        src/system.cpp \
        src/threadpool.cpp \
        src/timefn.cpp \
        src/ui.cpp \
        src/unicode.cpp \
        src/unpack.cpp \
        src/volume.cpp
    HEADERS += \
        src/rar.hpp
    RC_FILE = src/dll.rc

    win32:LIBS += -lshell32 -ladvapi32 -luser32
}
