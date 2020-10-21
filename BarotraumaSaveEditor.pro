#-------------------------------------------------
#
# Project created by QtCreator 2020-10-17T00:56:31
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BarotraumaSaveEditor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

QMAKE_LFLAGS_RELEASE += -static

INCLUDEPATH += vendor

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    gamesessioneditor.cpp \
    saveutil.cpp \
    gamesession.cpp

HEADERS += \
        mainwindow.h \
    gamesessioneditor.h \
    saveutil.h \
    vendor/gzip-cpp/decompress.hpp \
    vendor/gzip-cpp/config.hpp \
    vendor/gzip-cpp/compress.hpp \
    fileutils.h \
    gamesession.h

FORMS += \
        mainwindow.ui \
    gamesessioneditor.ui

LIBS += -lz

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
