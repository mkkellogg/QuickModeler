QT += widgets qml quick 
CONFIG += c++11

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/RenderSurface.h \
    $$PWD/RendererGL.h \
    $$PWD/Demo.h \
    $$PWD/MouseHandler.h

SOURCES += \
    $$PWD/RenderSurface.cpp \
    $$PWD/RendererGL.cpp \
    $$PWD/Main.cpp \
    $$PWD/Demo.cpp \
    $$PWD/MouseHandler.cpp

RESOURCES += \
    $$PWD/qml/qml.qrc

