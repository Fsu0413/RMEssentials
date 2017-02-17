
include($$PWD/../../RMEssentials.pri)

QT = core gui network

TEMPLATE = lib

CONFIG -= staticlib
CONFIG += plugin

LIBS += -L$$OUT_PWD/../../../dist/lib -lRMEss
INCLUDEPATH += $$OUT_PWD/../../../dist/include
