#-------------------------------------------------
#
# Project created by QtCreator 2022-12-05T14:31:55
#
#-------------------------------------------------

QT       += core gui
#QT       += serialport
QT       += widgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FATool
TEMPLATE = app
#RC_ICONS = Logo.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        PhysicalDriveInfo.cpp\
        CBadBlkView.cpp \
        FolderOperation.cpp \
        CommonFunc.cpp \
        DebugWindows.cpp


HEADERS += \
        mainwindow.h \
        PhysicalDriveInfo.h\
        CBadBlkView.h \
        FolderOperation.h \
        CommonFunc.h \
        DebugWindows.h


unix{
INCLUDEPATH += ./util/

HEADERS += \
    util/argconfig.h \
    util/json.h \
    util/parser.h \
    util/suffix.h \
    linux/cmd.h \
    linux/cmd_handler.h \
    linux/common.h \
    linux/define_cmd.h \
#    linux/fabrics.h \
    linux/lightnvm.h \
    linux/nvme-builtin.h \
    linux/nvme-ioctl.h \
    linux/nvme-lightnvm.h \
    linux/nvme-models.h \
    linux/nvme-print.h \
    linux/nvme-status.h \
    linux/nvme.h \
    linux/nvme_base.h \
    linux/nvme_cpp.h \
    linux/nvme_ioctl.h \
    linux/plugin.h \
    pci/config.h \
    pci/header.h \
    pci/pci.h \
    pci/types.h \

LIBS += -lpci
LIBS += -ldl

SOURCES += \
   ./util/argconfig.c \
   ./util/json.c   \
   ./util/parser.c \
   ./util/suffix.c \
#    linux/fabrics.c \
    linux/nvme-filters.c \
    linux/nvme-ioctl.c \
    linux/nvme-lightnvm.c \
    linux/nvme-models.c \
    linux/nvme-print.c \
    linux/nvme-status.c \
    linux/nvme-topology.c \
    linux/nvme.c \
    linux/plugin.c \
}

win32{
QT      += axcontainer
HEADERS += \
#    windows/CmdInterface.h \
#    windows/FirmwareUpdate.h \
    windows/NvmeApp.h \
    windows/NvmeCmd.h \
    windows/Nvmeioctl.h \
    windows/StorageQuery.h \
    windows/StorQuery.h \
    windows/SlotSpeedGetter.h \

SOURCES += \
#    windows/CmdInterface.cpp \
#    windows/FirmwareUpdate.cpp \
    windows/NvmeApp.cpp \
    windows/NvmeCmd.cpp \
    windows/SlotSpeedGetter.cpp \

LIBS += -lsetupapi
}

FORMS += \
        mainwindow.ui \
    debugwindows.ui
CONFIG += console

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    image/etc.qrc \

#DISTFILES += \
#    res.rc



