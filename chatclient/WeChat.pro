QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#程序版本
VERSION  = 1.0
#程序图标
RC_ICONS = images/icon/logo.ico
#产品名称
QMAKE_TARGET_PRODUCT = WeChatClient
#版权所有
QMAKE_TARGET_COPYRIGHT = haojoy
#文件说明
QMAKE_TARGET_DESCRIPTION = WeChatClient

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    logindialog.h \
    mainwindow.h

FORMS += \
    logindialog.ui \
    mainwindow.ui

LIBS += -lws2_32


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

include($$PWD/Net/Net.pri)
include($$PWD/Controls/Controls.pri)
include($$PWD/Common/Common.pri)
include($$PWD/Models/Models.pri)
include($$PWD/Views/Views.pri)
