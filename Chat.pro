#-------------------------------------------------
#
# Project created by QtCreator 2016-07-26T15:12:23
#
#-------------------------------------------------

QT       += core gui
QT += network
CONFIG += c++11
QT += multimedia
QT += core
QT += multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Chat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    findterminal.cpp \
    mylistwidget.cpp \
    textchat.cpp \
    videodisplay.cpp \
    myextextedit.cpp \
    transferfile.cpp \
    transferpic.cpp

HEADERS  += mainwindow.h \
    findterminal.h \
    mylistwidget.h \
    textchat.h \
    videodisplay.h \
    msginfo.h \
    myextextedit.h \
    transferfile.h \
    transferpic.h

FORMS    += mainwindow.ui

RESOURCES += \
    picture.qrc
