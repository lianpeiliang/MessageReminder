QT += core gui widgets

TEMPLATE = app
TARGET = MessageReminder
CONFIG += c++11

SOURCES += main.cpp \
           toastwidget.cpp
HEADERS += toastwidget.h

msvc:QMAKE_CXXFLAGS += /utf-8

