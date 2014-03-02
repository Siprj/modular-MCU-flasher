#-------------------------------------------------
#
# Project created by QtCreator 2014-02-28T22:10:39
#
#-------------------------------------------------

QT       += widgets serialport

TARGET = dmxFlasher
TEMPLATE = lib

SOURCES += dmxflasher.cpp \
    widgetwithnotifi.cpp \
    comboboxwithpopupnotifi.cpp \
    DmxBootProtocol.cpp

HEADERS += dmxflasher.h \
    globalStatic.h \
    widgetwithnotifi.h \
    comboboxwithpopupnotifi.h \
    DmxBootProtocol.h

DESTDIR = ../../../complete/plugins

OTHER_FILES += \
    dmxFlasher.json

FORMS += \
    dmxFlasher.ui
