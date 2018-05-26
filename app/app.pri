QT += widgets qml quick 
CONFIG += c++11

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/renderergl.h \
    $$PWD/rendersurface.h \
    $$PWD/demo.h

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/renderergl.cpp \
    $$PWD/rendersurface.cpp \
    $$PWD/demo.cpp

RESOURCES += \
    $$PWD/qml/qml.qrc

