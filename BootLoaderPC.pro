#-------------------------------------------------
#
# Project created by QtCreator 2013-12-10T14:16:05
#
#-------------------------------------------------

QT  += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = BootLoaderPC
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    DmxBootProtocol.cpp \
    HexReader.cpp \
    settingsdialog.cpp


HEADERS  += mainwindow.h \
    DmxBootProtocol.h \
    HexReader.h \
    settingsdialog.h

OTHER_FILES += \
    doc/BootLoaderPC.qdocconf  \
    doc/qt-html-templates-offline.qdocconf \
    doc/qt-defines.qdocconf \
    doc/qt-cpp-ignore.qdocconf \
    doc/manifest-meta.qdocconf \
    doc/macros.qdocconf \
    doc/compat.qdocconf \
    doc/docSrc/main.qdoc

FORMS    += mainwindow.ui \
    settingsdialog.ui

DEFINES += TEST

RESOURCES += resource.qrc
