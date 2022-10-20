QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatclientform.cpp \
    chatserverform.cpp \
    clientitem.cpp \
    clientmanagerform.cpp \
    main.cpp \
    mainwindow.cpp \
    productitem.cpp \
    productmanagerform.cpp \
    shopitem.cpp \
    shoplistmanagerform.cpp

HEADERS += \
    chatclientform.h \
    chatserverform.h \
    clientitem.h \
    clientmanagerform.h \
    mainwindow.h \
    productitem.h \
    productmanagerform.h \
    shopitem.h \
    shoplistmanagerform.h

FORMS += \
    chatserverform.ui \
    clientmanagerform.ui \
    mainwindow.ui \
    productmanagerform.ui \
    shoplistmanagerform.ui

TRANSLATIONS += \
    CSApp_ko_KR.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
