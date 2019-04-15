TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

include(..\common.pri)

SOURCES += main.cpp \
    myhog.cpp

HEADERS += \
    myhog.h
