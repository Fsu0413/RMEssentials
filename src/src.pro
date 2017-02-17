include($$_PRO_FILE_PWD_/../RMEssentials.pri)

TEMPLATE = subdirs

rmessentials.depends = librmessentials
plugins.depends = librmessentials

SUBDIRS = librmessentials rmessentials plugins
