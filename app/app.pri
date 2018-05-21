QT += widgets qml quick 
CONFIG += c++11

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/renderergl.h

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/renderergl.cpp

RESOURCES += \
    $$PWD/qml/qml.qrc

