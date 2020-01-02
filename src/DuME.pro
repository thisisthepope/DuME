#-------------------------------------------------
#
# Project created by QtCreator 2017-10-04T20:46:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DuME
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

# The current version
DEFINES += APPVERSION=\\\"0.1.0-Alpha1\\\"

# For speeding up launch time when working, you can comment this line out.
# Be sure to set it back when you're finished, before committing your changes.
DEFINES += INIT_AE INIT_FFMPEG


SOURCES += \
    AfterEffects/aftereffects.cpp \
    AfterEffects/aftereffectsversion.cpp \
    Renderer/abstractrenderer.cpp \
    AfterEffects/aerenderer.cpp \
    Renderer/abstractrendererinfo.cpp \
    Renderer/queueitem.cpp \
    Renderer/renderqueue.cpp \
    FFmpeg/ffmpegrenderer.cpp \
    UI/Blocks/blockaecomp.cpp \
    UI/Blocks/blockalpha.cpp \
    UI/Blocks/blockaudiobitrate.cpp \
    UI/Blocks/blockaudiocodec.cpp \
    UI/Blocks/blockcolor.cpp \
    UI/Blocks/blockcustom.cpp \
    UI/Blocks/blockexr.cpp \
    UI/Blocks/blockframerate.cpp \
    UI/Blocks/blockloops.cpp \
    UI/Blocks/blockpixformat.cpp \
    UI/Blocks/blockresize.cpp \
    UI/Blocks/blocksampling.cpp \
    UI/Blocks/blockstartnumber.cpp \
    UI/Blocks/blockvideobitrate.cpp \
    UI/Blocks/blockvideocodec.cpp \
    UI/Blocks/blockvideoprofile.cpp \
    UI/Blocks/uiblockcontent.cpp \
    UI/Blocks/uiblockwidget.cpp \
    UI/uibigcombobox.cpp \
    main.cpp \
    Renderer/mediainfo.cpp \
    FFmpeg/ffmpeg.cpp \
    FFmpeg/ffcodec.cpp \
    FFmpeg/ffmuxer.cpp \
    FFmpeg/ffpixformat.cpp \
    FFmpeg/ffbaseobject.cpp \
    UI/uidropshadow.cpp \
    UI/uiframeless.cpp \
    UI/uiinputwidget.cpp \
    UI/uimainwindow.cpp \
    UI/uioutputwidget.cpp \
    UI/uiqueuewidget.cpp \
    UI/uirainbox.cpp \
    UI/uisettingswidget.cpp \
    UI/uitoolbarspacer.cpp \
    utils.cpp

HEADERS += \
    AfterEffects/aftereffects.h \
    AfterEffects/aftereffectsversion.h \
    Renderer/abstractrenderer.h \
    AfterEffects/aerenderer.h \
    Renderer/abstractrendererinfo.h \
    Renderer/queueitem.h \
    Renderer/renderqueue.h \
    FFmpeg/ffmpegrenderer.h \
    Renderer/mediainfo.h \
    FFmpeg/ffmpeg.h \
    FFmpeg/ffcodec.h \
    FFmpeg/ffmuxer.h \
    FFmpeg/ffpixformat.h \
    FFmpeg/ffbaseobject.h \
    UI/Blocks/blockaecomp.h \
    UI/Blocks/blockalpha.h \
    UI/Blocks/blockaudiobitrate.h \
    UI/Blocks/blockaudiocodec.h \
    UI/Blocks/blockcolor.h \
    UI/Blocks/blockexr.h \
    UI/Blocks/blockframerate.h \
    UI/Blocks/blockloops.h \
    UI/Blocks/blockpixformat.h \
    UI/Blocks/blockresize.h \
    UI/Blocks/blocksampling.h \
    UI/Blocks/blockstartnumber.h \
    UI/Blocks/blockvideobitrate.h \
    UI/Blocks/blockvideocodec.h \
    UI/Blocks/blockvideoprofile.h \
    UI/Blocks/blockcustom.h \
    UI/Blocks/uiblockcontent.h \
    UI/Blocks/uiblockwidget.h \
    UI/uibigcombobox.h \
    UI/uidropshadow.h \
    UI/uiframeless.h \
    UI/uiinputwidget.h \
    UI/uimainwindow.h \
    UI/uioutputwidget.h \
    UI/uiqueuewidget.h \
    UI/uirainbox.h \
    UI/uisettingswidget.h \
    UI/uitoolbarspacer.h

FORMS += \
    UI/Blocks/blockaecomp.ui \
    UI/Blocks/blockalpha.ui \
    UI/Blocks/blockaudiobitrate.ui \
    UI/Blocks/blockaudiocodec.ui \
    UI/Blocks/blockcolor.ui \
    UI/Blocks/blockexr.ui \
    UI/Blocks/blockframerate.ui \
    UI/Blocks/blockloops.ui \
    UI/Blocks/blockpixformat.ui \
    UI/Blocks/blockresize.ui \
    UI/Blocks/blocksampling.ui \
    UI/Blocks/blockstartnumber.ui \
    UI/Blocks/blockvideobitrate.ui \
    UI/Blocks/blockvideocodec.ui \
    UI/Blocks/blockvideoprofile.ui \
    UI/Blocks/blockcustom.ui \
    UI/Blocks/uiblockwidget.ui \
    UI/mainwindow.ui \
    UI/outputwidget.ui \
    UI/settingswidget.ui \
    UI/queuewidget.ui \
    UI/inputwidget.ui \
    UI/toolbarspacer.ui

RESOURCES += \
    resources.qrc

DISTFILES += \
    resources/ico.rc

RC_FILE = resources/ico.rc

# OpenImageIO

# unix:!macx: LIBS += -lOpenImageIO \
#    -lOpenImageIO_Util

#INCLUDEPATH += /usr/include/OpenImageIO
#DEPENDPATH += /usr/include/OpenImageIO
