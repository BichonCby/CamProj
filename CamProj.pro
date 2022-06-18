#-------------------------------------------------
#
# Project created by QtCreator 2022-06-14T17:23:21
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CamProj
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lqledplugin
SOURCES += \
        main.cpp \
        mainfenetre.cpp \
    mavideocapture.cpp

HEADERS += \
        mainfenetre.h \
    mavideocapture.h

FORMS += \
        mainfenetre.ui

# __________________________________________________
# Placé en premier, link en priorité avec les librairies contenues dans le répertoire d'installation de Qt (cas de plusieurs versions de librairies Qt installées sur la même machine dans des répertoires différents)
LIBS += -L$$[QT_INSTALL_LIBS]
# __________________________________________________
# Gestion du joystick
LIBS += -lsfml-graphics -lsfml-window -lsfml-system
# __________________________________________________
# Gestion des webcam et traitements video
win32 {
    LIBS += -LC:/win_progs/Qt/5.2.1/mingw48_32/plugins/designer -lqhexeditplugin -lqledplugin -lq7segplugin
    LIBS += -LC:/win_progs/OpenCV/opencv_2_4_9/bin -llibopencv_core249 -llibopencv_highgui249 -llibopencv_imgproc249
    INCLUDEPATH += "C:/win_progs/OpenCV/opencv_2_4_9/include"
}

linux {
    LIBS +=`pkg-config opencv --libs`  # Ligne de commande qui renvoie toutes les librairies opencv sous la forme "-lopencv... -lopencv..."
}

DISTFILES +=
