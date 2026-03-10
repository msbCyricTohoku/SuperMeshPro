# SuperMeshPro.pro

QT += core gui widgets opengl

greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets

TARGET = SuperMeshPro
TEMPLATE = app

CONFIG += c++17

INCLUDEPATH += /usr/include/eigen3

SOURCES += \
    src/FEASolver.cpp \
    src/HeatTransfer.cpp \
    src/RayTracingOptic.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/MeshRenderer.cpp \
    src/MeshTopology.cpp \
    src/SubdivisionAlgorithms.cpp

HEADERS += \
    src/FEASolver.h \
    src/HeatTransfer.h \
    src/RayTracingOptic.h \
    src/mainwindow.h \
    src/MeshRenderer.h \
    src/MeshTopology.h \
    src/SubdivisionAlgorithms.h

# UI files
FORMS += \
    src/mainwindow.ui

RESOURCES += \
    src/resources.qrc

# Installation paths
target.path = /usr/bin

# Desktop file installation
desktop.path = /usr/share/applications
desktop.files = deployment/supermeshpro.desktop

# Icon installation
icon.path = /usr/share/icons/hicolor/64x64/apps
icon.files = src/resources/icon.png

INSTALLS += target desktop icon

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
