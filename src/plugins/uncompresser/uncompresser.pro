
include($$_PRO_FILE_PWD_/../../../RMEssentials.pri)

TEMPLATE = subdirs
SUBDIRS = libunrar libquazip

!winrt:!ios: SUBDIRS += unrarcli unzipcli 7zcli

# no plan to implement
# SUBDIRS += tarcli
