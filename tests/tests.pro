QT += widgets testlib
CONFIG += c++11 console testcase
TEMPLATE = app
TARGET = toastwidget_tests
SOURCES += tst_toastwidget.cpp ../toastwidget.cpp
HEADERS += ../toastwidget.h
