#-------------------------------------------------
#
# Project created by QtCreator 2014-02-28T22:00:42
#
#-------------------------------------------------

QT       += widgets


TARGET = hexReader
TEMPLATE = lib

SOURCES += hexreader.cpp \
    hexReaderCore.cpp

HEADERS += hexreader.h \
    globalStatic.h \
    hexReaderCore.h

DEFINES += \
#        TRACE_FUNCTION_CALLS

DESTDIR = ../../../complete/plugins
