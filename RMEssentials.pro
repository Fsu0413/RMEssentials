#-------------------------------------------------
#
# Project created by QtCreator 2014-11-15T20:35:05
#
#-------------------------------------------------

QT       += core gui widgets network gui-private

TARGET = RMEssentials
TEMPLATE = app



CONFIG += mobility
MOBILITY = 

RESOURCES += \
    lang.qrc

TRANSLATIONS += changename.ts

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

win32{
    RC_FILE += res/icon.rc
}

mac{
    ICON = res/1.icns
}

DEFINES += QUAZIP_STATIC XLSX_NO_LIB

HEADERS += \
    src/core/downloader.h \
    src/core/renamer.h \
    src/core/songstruct.h \
    src/core/uncompresser.h \
    src/core/utils.h \
    src/dialog/ChangeNameDialog.h \
    src/dialog/DownloadDialog.h \
    src/dialog/maindialog.h \
    src/dialog/SongClientEditDialog.h \
    src/quazip/crypt.h \
    src/quazip/ioapi.h \
    src/quazip/JlCompress.h \
    src/quazip/quaadler32.h \
    src/quazip/quachecksum32.h \
    src/quazip/quacrc32.h \
    src/quazip/quagzipfile.h \
    src/quazip/quaziodevice.h \
    src/quazip/quazip.h \
    src/quazip/quazip_global.h \
    src/quazip/quazipdir.h \
    src/quazip/quazipfile.h \
    src/quazip/quazipfileinfo.h \
    src/quazip/quazipnewinfo.h \
    src/quazip/unzip.h \
    src/quazip/zconf.h \
    src/quazip/zip.h \
    src/quazip/zlib.h \
    src/qtxlsx/xlsxabstractooxmlfile.h \
    src/qtxlsx/xlsxabstractooxmlfile_p.h \
    src/qtxlsx/xlsxabstractsheet.h \
    src/qtxlsx/xlsxabstractsheet_p.h \
    src/qtxlsx/xlsxcell.h \
    src/qtxlsx/xlsxcell_p.h \
    src/qtxlsx/xlsxcellformula.h \
    src/qtxlsx/xlsxcellformula_p.h \
    src/qtxlsx/xlsxcellrange.h \
    src/qtxlsx/xlsxcellreference.h \
    src/qtxlsx/xlsxchart.h \
    src/qtxlsx/xlsxchart_p.h \
    src/qtxlsx/xlsxchartsheet.h \
    src/qtxlsx/xlsxchartsheet_p.h \
    src/qtxlsx/xlsxcolor_p.h \
    src/qtxlsx/xlsxconditionalformatting.h \
    src/qtxlsx/xlsxconditionalformatting_p.h \
    src/qtxlsx/xlsxcontenttypes_p.h \
    src/qtxlsx/xlsxdatavalidation.h \
    src/qtxlsx/xlsxdatavalidation_p.h \
    src/qtxlsx/xlsxdocpropsapp_p.h \
    src/qtxlsx/xlsxdocpropscore_p.h \
    src/qtxlsx/xlsxdocument.h \
    src/qtxlsx/xlsxdocument_p.h \
    src/qtxlsx/xlsxdrawing_p.h \
    src/qtxlsx/xlsxdrawinganchor_p.h \
    src/qtxlsx/xlsxformat.h \
    src/qtxlsx/xlsxformat_p.h \
    src/qtxlsx/xlsxglobal.h \
    src/qtxlsx/xlsxmediafile_p.h \
    src/qtxlsx/xlsxnumformatparser_p.h \
    src/qtxlsx/xlsxrelationships_p.h \
    src/qtxlsx/xlsxrichstring.h \
    src/qtxlsx/xlsxrichstring_p.h \
    src/qtxlsx/xlsxsharedstrings_p.h \
    src/qtxlsx/xlsxsimpleooxmlfile_p.h \
    src/qtxlsx/xlsxstyles_p.h \
    src/qtxlsx/xlsxtheme_p.h \
    src/qtxlsx/xlsxutility_p.h \
    src/qtxlsx/xlsxworkbook.h \
    src/qtxlsx/xlsxworkbook_p.h \
    src/qtxlsx/xlsxworksheet.h \
    src/qtxlsx/xlsxworksheet_p.h \
    src/qtxlsx/xlsxzipreader_p.h \
    src/qtxlsx/xlsxzipwriter_p.h

SOURCES += \
    src/core/downloader.cpp \
    src/core/main.cpp \
    src/core/renamer.cpp \
    src/core/songstruct.cpp \
    src/core/uncompresser.cpp \
    src/core/utils.cpp \
    src/dialog/ChangeNameDialog.cpp \
    src/dialog/DownloadDialog.cpp \
    src/dialog/maindialog.cpp \
    src/dialog/SongClientEditDialog.cpp \
    src/quazip/JlCompress.cpp \
    src/quazip/qioapi.cpp \
    src/quazip/quaadler32.cpp \
    src/quazip/quacrc32.cpp \
    src/quazip/quagzipfile.cpp \
    src/quazip/quaziodevice.cpp \
    src/quazip/quazip.cpp \
    src/quazip/quazipdir.cpp \
    src/quazip/quazipfile.cpp \
    src/quazip/quazipfileinfo.cpp \
    src/quazip/quazipnewinfo.cpp \
    src/quazip/unzip.c \
    src/quazip/zip.c \
    src/qtxlsx/xlsxabstractooxmlfile.cpp \
    src/qtxlsx/xlsxabstractsheet.cpp \
    src/qtxlsx/xlsxcell.cpp \
    src/qtxlsx/xlsxcellformula.cpp \
    src/qtxlsx/xlsxcellrange.cpp \
    src/qtxlsx/xlsxcellreference.cpp \
    src/qtxlsx/xlsxchart.cpp \
    src/qtxlsx/xlsxchartsheet.cpp \
    src/qtxlsx/xlsxcolor.cpp \
    src/qtxlsx/xlsxconditionalformatting.cpp \
    src/qtxlsx/xlsxcontenttypes.cpp \
    src/qtxlsx/xlsxdatavalidation.cpp \
    src/qtxlsx/xlsxdocpropsapp.cpp \
    src/qtxlsx/xlsxdocpropscore.cpp \
    src/qtxlsx/xlsxdocument.cpp \
    src/qtxlsx/xlsxdrawing.cpp \
    src/qtxlsx/xlsxdrawinganchor.cpp \
    src/qtxlsx/xlsxformat.cpp \
    src/qtxlsx/xlsxmediafile.cpp \
    src/qtxlsx/xlsxnumformatparser.cpp \
    src/qtxlsx/xlsxrelationships.cpp \
    src/qtxlsx/xlsxrichstring.cpp \
    src/qtxlsx/xlsxsharedstrings.cpp \
    src/qtxlsx/xlsxsimpleooxmlfile.cpp \
    src/qtxlsx/xlsxstyles.cpp \
    src/qtxlsx/xlsxtheme.cpp \
    src/qtxlsx/xlsxutility.cpp \
    src/qtxlsx/xlsxworkbook.cpp \
    src/qtxlsx/xlsxworksheet.cpp \
    src/qtxlsx/xlsxzipreader.cpp \
    src/qtxlsx/xlsxzipwriter.cpp

INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/quazip
INCLUDEPATH += src/qtxlsx

LIBS += -L.

