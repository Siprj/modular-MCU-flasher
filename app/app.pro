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
        mainwindow.cpp


HEADERS  += mainwindow.h \
    interfaces/FlasherPluginInterface.h \
    interfaces/ReaderPluginInterface.h \
    interfaces/trace.h

OTHER_FILES += \
    doc/BootLoaderPC.qdocconf  \
    doc/qt-html-templates-offline.qdocconf \
    doc/qt-defines.qdocconf \
    doc/qt-cpp-ignore.qdocconf \
    doc/manifest-meta.qdocconf \
    doc/macros.qdocconf \
    doc/compat.qdocconf \
    doc/docSrc/main.qdoc

FORMS    += mainwindow.ui

DEFINES += \
#           TRACE_FUNCTION_CALLS

RESOURCES += resource.qrc

DESTDIR = ../complete/
