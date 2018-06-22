TEMPLATE = app
TARGET = Modeler
QT += qml quick opengl

include(app.pri)

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

DEFINES += GL_GLEXT_PROTOTYPES


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../Core/build/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../Core/build/debug/ -lcore
else:unix: LIBS += -L$$PWD/../../../Core/build/ -lcore

INCLUDEPATH += $$PWD/../../../Core/build/include/
DEPENDPATH += $$PWD/../../../Core/build/include/

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../Core/build/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../Core/build/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../Core/build/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../Core/build/debug/core.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../Core/build/libcore.a




unix|win32: LIBS += -lpng





win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../assimp_dynamic/build/code/release/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../assimp_dynamic/build/code/ -lassimp
else:unix: LIBS += -L/home/mark/Development/assimp_dynamic/build/code/ -lassimp

INCLUDEPATH += $$PWD/../../../assimp_dynamic/include
DEPENDPATH += $$PWD/../../../assimp_dynamic/include
