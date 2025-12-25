QT += core gui svg mqtt quick bluetooth sql network
QT += multimedia
QT += texttospeech

include(D:\Air\QXlsx\QXlsx\QXlsx.pri)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = multilangtest
CONFIG += c++17


QMAKE_TARGET_PRODUCT = multilangtest
QMAKE_PROJECT_DEPTH = 0

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += D:\Air\QXlsx\QXlsx\header

SOURCES += \
    QrCodeGenerator.cpp \
    fullscrdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    qrcodegen.cpp \
    testdialog.cpp \
    tinyxml2.cpp

HEADERS += \
    fullscrdialog.h \
    mainwindow.h \
    qrcodegen.h \
    testdialog.h \
    tinyxml2.h

FORMS += \
    fullscrdialog.ui \
    mainwindow.ui \
    testdialog.ui

TRANSLATIONS += \
    multilangtest_ja_JP.ts \
    multilangtest_zh_CN.ts\
    multilangtest_en.ts

#CONFIG += lrelease
CONFIG += embed_translations

win32 {
DEFINES += _CRT_SECURE_NO_WARNINGS
LIBS += -L$$PWD -lzint
#LIBS += -lpthread libwsock32 libws2_32
}

RESOURCES += \
    res.qrc \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

PLUGIN_TYPE = sqldrivers


