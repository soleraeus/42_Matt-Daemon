######################################################################
# Automatically generated by qmake (3.1) Fri Oct 13 19:44:47 2023
######################################################################

openssl.target = ../libs/openss-3.0.11/libcrypto.a
openssl.commands = test -d ../libs/openssl-3.0.11 || (cd ../libs; tar -xvf openssl-3.0.11.tar.gz);cd ../libs/openssl-3.0.11; ./config no-idea no-camellia no-seed no-bf no-cast no-des no-rc2 no-rc4 no-rc5 no-md2 no-md4 no-mdc2 no-dsa no-dh no-ec no-ecdsa no-ecdh no-sock no-ssl3 no-err no-engine; make depend; make build_generated libcrypto.a

QMAKE_EXTRA_TARGETS += openssl
PRE_TARGETDEPS += ../libs/openss-3.0.11/libcrypto.a
TEMPLATE = app
TARGET = Ben_AFK
INCLUDEPATH += .
LIBS += ../libs/openssl-3.0.11/libcrypto.a

# You can make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# Please consult the documentation of the deprecated API in order to know
# how to port your code away from it.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000 # disables all APIs deprecated in Qt 6.0.0 and earlier
QT += core gui widgets network
# Input
HEADERS += mainwindow.h ui_mainwindow.h secured_client.hpp
FORMS += mainwindow.ui
SOURCES += main.cpp mainwindow.cpp secured_client.cpp
