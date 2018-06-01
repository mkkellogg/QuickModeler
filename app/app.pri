QT += widgets qml quick 
CONFIG += c++11

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/RenderSurface.h \
    $$PWD/RendererGL.h \
    $$PWD/ModelerApp.h \
    $$PWD/ModelerAppWindow.h \
    $$PWD/MouseAdapter.h \
    $$PWD/GestureAdapter.h \
    $$PWD/EventQueue.h \
    $$PWD/Event.h \
    $$PWD/Types.h \
    $$PWD/PipedEventAdapter.h

SOURCES += \
    $$PWD/RenderSurface.cpp \
    $$PWD/RendererGL.cpp \
    $$PWD/Main.cpp \
    $$PWD/ModelerApp.cpp \
    $$PWD/ModelerAppWindow.cpp \
    $$PWD/MouseAdapter.cpp \
    $$PWD/GestureAdapter.cpp \
    $$PWD/Event.cpp

RESOURCES += \
    $$PWD/qml/qml.qrc

