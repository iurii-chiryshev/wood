#-------------------------------------------------
#
# Project created by QtCreator 2017-01-13T11:05:11
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = peeler
TEMPLATE = app


include(..\common.pri)



SOURCES += \
    util/pcl.cpp \
    util/osg.cpp \
    ui/osgviewerwidget.cpp \
    async/backgroundworker.cpp \
    async/renderthread.cpp \
    async/task.cpp \
    fitting/model.cpp \
    fitting/modelcylinder.cpp \
    fitting/fitter.cpp \
    ui/osgmultiviewerwidget.cpp \
    fitting/modelcone.cpp \
    ui/mainwindow.cpp \
    main.cpp \
    io/baseloader.cpp \
    io/fileloader.cpp \
    io/modelloader.cpp \
    io/scenemaker.cpp


HEADERS  += \
    util/pcl.h \
    util/osg.h \
    ui/osgviewerwidget.h \
    async/backgroundworker.h \
    async/renderthread.h \
    async/task.h \
    fitting/model.h \
    fitting/modelcylinder.h \
    fitting/fitter.h \
    ui/osgmultiviewerwidget.h \
    fitting/modelcone.h \
    ui/mainwindow.h \
    core/singleton.h \
    io/baseloader.h \
    io/fileloader.h \
    io/modelloader.h \
    io/scenemaker.h


FORMS    += \
    ui/mainwindow.ui

RESOURCES += \
    icons.qrc

DISTFILES += \
    icons/3d_model_64.png \
    icons/Curved_Pipe-64.png \
    icons/done.png \
    icons/fit_to_window.png \
    icons/normal_size.png \
    icons/opencv.png \
    icons/terminal.png \
    icons/tree.png \
    icons/wood.png \
    icons/wood2_64.png \
    icons/wood_64.png \
    icons/zoom_in.png \
    icons/zoom_out.png \
    icons/application_cascade.ico \
    icons/application_tile_horizontal.ico \
    icons/floppy.ico \
    icons/qt.ico




