#-------------------------------------------------
#
# Project created by QtCreator 2015-02-02T13:22:28
#
#-------------------------------------------------

QT       -= core gui

TARGET = houio
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    src/Attribute.cpp \
    src/Field.cpp \
    src/math/Color.cpp \
    src/math/Math.cpp \
    src/json.cpp \
    src/HouGeoAdapter.cpp \
    src/HouGeo.cpp \
    src/HouGeoIO.cpp \
    src/Geometry.cpp

HEADERS += \
    include/houio/Attribute.h \
    include/houio/Field.h \
    include/houio/HouGeo.h \
    include/houio/HouGeoAdapter.h \
    include/houio/HouGeoIO.h \
    include/houio/HouScene.h \
    include/houio/ImportHoudini.h \
    include/houio/json.h \
    include/houio/types.h \
    include/houio/math/BoundingBox2.h \
    include/houio/math/BoundingBox3.h \
    include/houio/math/Color.h \
    include/houio/math/Math.h \
    include/houio/math/Matrix22.h \
    include/houio/math/Matrix22Algo.h \
    include/houio/math/Matrix33.h \
    include/houio/math/Matrix33Algo.h \
    include/houio/math/Matrix44.h \
    include/houio/math/Matrix44Algo.h \
    include/houio/math/Ray3.h \
    include/houio/math/RNG.h \
    include/houio/math/Vec2.h \
    include/houio/math/Vec2Algo.h \
    include/houio/math/Vec3.h \
    include/houio/math/Vec3Algo.h \
    include/houio/math/Vec4.h \
    include/houio/math/Vec4Algo.h \
    include/houio/Geometry.h


INCLUDEPATH += include

unix {
    target.path = /usr/lib
    INSTALLS += target
}
