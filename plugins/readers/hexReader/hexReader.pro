#-------------------------------------------------
#
# Project created by QtCreator 2014-02-28T22:00:42
#
#-------------------------------------------------

QT       += widgets

TARGET = hexReader
TEMPLATE = lib

DEFINES += HEXREADER_LIBRARY

SOURCES += hexreader.cpp

HEADERS += hexreader.h\
        hexreader_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
