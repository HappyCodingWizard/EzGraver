#-------------------------------------------------
#
# Project created by QtCreator 2016-08-22T09:16:50
#
#-------------------------------------------------

QT += core
QT += gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EzGraverUi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    clicklabel.cpp

HEADERS  += mainwindow.h \
    clicklabel.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../EzGraverLib/release/ -lEzGraverLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../EzGraverLib/debug/ -lEzGraverLib
else:unix: LIBS += -L$$OUT_PWD/../EzGraverLib/ -lEzGraverLib

INCLUDEPATH += $$PWD/../EzGraverLib
DEPENDPATH += $$PWD/../EzGraverLib