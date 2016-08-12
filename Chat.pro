#-------------------------------------------------
#
# Project created by QtCreator 2016-07-26T15:12:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += network
CONFIG += c++11
CONFIG+=thread
QT += multimedia
QT += core
QT += multimediawidgets

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

INCLUDEPATH +=  ffmpeg/include
INCLUDEPATH +=  x264/include

LIBS += ./ffmpeg/lib/libavcodec.dll.a \
        ./ffmpeg/lib/libavfilter.dll.a \
        ./ffmpeg/lib/libavformat.dll.a \
        ./ffmpeg/lib/libswscale.dll.a \
        ./ffmpeg/lib/libavutil.dll.a \

LIBS += ./x264/lib/libx264.dll.a

#设置目标生成目录，注意，此时程序运行加载的资源也要放到此目录（qss/emoji）
DESTDIR=bin
#打开目录并选中

LIBS += -lshell32
LIBS +=  -lWs2_32

FORMS    += mainwindow.ui

RESOURCES += \
    picture.qrc
