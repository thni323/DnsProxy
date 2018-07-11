#-------------------------------------------------
#
# Project created by QtCreator 2015-10-04T20:09:01
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DnsProxy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    dnsproxy.cpp

HEADERS  += mainwindow.h \
    dnsproxy.h

FORMS    += mainwindow.ui
