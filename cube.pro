QT += core gui widgets opengl

TARGET = cube
TEMPLATE = app

SOURCES += src/source/main.cpp

SOURCES += \
    src/source/mainwidget.cpp \
    src/source/geometryengine.cpp

HEADERS += \
    src/header/mainwidget.h \
    src/header/geometryengine.h

RESOURCES += \
    src/ressource/shaders.qrc \
    src/ressource/textures.qrc
