include($$_PRO_FILE_PWD_/../RMEssentials.pri)

TEMPLATE = subdirs

rmessentials.depends = librmessentials

SUBDIRS = librmessentials rmessentials

use_quazip {
    librmessentials.depends = quazip
    SUBDIRS += quazip
}
