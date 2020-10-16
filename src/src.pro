#-------------------------------------------------
#
# ukui-system-monitor
#
#-------------------------------------------------

QT += core x11extras gui dbus network

isEqual(QT_MAJOR_VERSION, 5) {
    QT += widgets gui svg x11extras
}

include(QtSingleApplication/qtsingleapplication.pri)

TARGET = ukui-system-monitor
TEMPLATE = app
DESTDIR = ..

LIBS += -L/usr/lib/ -lX11 -lpcap -lm

CONFIG        += link_pkgconfig \
                 C++11
#gio-2.0
#LIBS +=-lgio-2.0 -lglib-2.0

CONFIG +=  c++11\
           link_pkgconfig
PKGCONFIG += libgtop-2.0 \
             libsystemd \
             gsettings-qt \
             x11

ICON.files = img/ukui-system-monitor.png
ICON.path = /usr/share/icons/hicolor/

desktop.files += ../ukui-system-monitor.desktop
desktop.path = /usr/share/applications/

target.source += $$TARGET
target.path = /usr/bin

schemes.files += ../data/*.xml
schemes.path = /usr/share/glib-2.0/schemas/

#include(src/QtSingleApplication/qtsingleapplication.pri)
#include(qtsingleapplication/qtsingleapplication.pri)
#INCLUDEPATH += qtsingleapplication
#DEPENDPATH += qtsingleapplication

INSTALLS += target \
            desktop \
            schemes \
            ICON

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

HEADERS += \
    shell/customstyle.h \
    ../shell/macro.h \
    systemmonitor.h \
    processmanager.h \
    processdialog.h \
    processlistwidget.h \
    processlistitem.h \
    processworker.h \
    linebandwith.h \
    util.h \
    ../component/utils.h \
    ../widgets/mydialog.h \
    ../widgets/mytristatebutton.h \
    ../widgets/myunderlinebutton.h \
    ../widgets/myhoverbutton.h \
    propertiesdialog.h \
    processcategory.h \
    processdata.h \
    ../widgets/myactiongroup.h \
    ../widgets/myactiongroupitem.h \
    ../widgets/myaction.h \
    monitortitlewidget.h \
    resourcesdialog.h \
    filesystemdialog.h \
    filesystemworker.h \
    filesystemdata.h \
    ../widgets/mysearchedit.h \
    networkwidget.h \
    networkflow.h \
    netcatogoryshow.h \
    cpuratewidget.h \
    cpuballwidget.h \
    smoothcurvegenerator.h \
    memorywidget.h \
    memorycircle.h \
    networkindicator.h \
    resourcesindicator.h \
    resourcescategory.h \
    filesystemlistwidget.h \
    filesystemlistitem.h \
    filesystemwatcher.h \
    renicedialog.h \
    ../component/mytitlebar.h \
    framelessExtended/cursorposcalculator.h \
    framelessExtended/framelesshandle.h \
    framelessExtended/framelesshandleprivate.h \
    framelessExtended/widgethandlerealize.h \
    maincontroller.h \
    singleProcessNet/devices.h \
    singleProcessNet/scanthread.h \
    singleProcessNet/packet.h \
    singleProcessNet/kylinsystemnethogs.h \
    singleProcessNet/process.h \
    singleProcessNet/connection.h \
    singleProcessNet/conninode.h \
    singleProcessNet/inode2prog.h \
    singleProcessNet/refreshthread.h \
    singleProcessNet/decpcap.h
#    newresourcesdialog.h

SOURCES += \
    main.cpp \
    shell/customstyle.cpp \
    systemmonitor.cpp \
    processmanager.cpp \
    processdialog.cpp \
    processlistwidget.cpp \
    processlistitem.cpp \
    processworker.cpp \
    linebandwith.cpp \
    util.cpp \
    ../widgets/mydialog.cpp \
    ../widgets/mytristatebutton.cpp \
    ../widgets/myunderlinebutton.cpp \
    ../widgets/myhoverbutton.cpp \
    propertiesdialog.cpp \
    processcategory.cpp \
    ../widgets/myactiongroup.cpp \
    ../widgets/myactiongroupitem.cpp \
    ../widgets/myaction.cpp \
    monitortitlewidget.cpp \
    resourcesdialog.cpp \
    filesystemdialog.cpp \
    filesystemworker.cpp \
    filesystemdata.cpp \
    ../widgets/mysearchedit.cpp \
    networkwidget.cpp \
    networkflow.cpp \
    netcatogoryshow.cpp \
    cpuratewidget.cpp \
    cpuballwidget.cpp \
    smoothcurvegenerator.cpp \
    memorywidget.cpp \
    memorycircle.cpp \
    networkindicator.cpp \
    resourcesindicator.cpp \
    resourcescategory.cpp \
    filesystemlistwidget.cpp \
    filesystemlistitem.cpp \
    filesystemwatcher.cpp \
    renicedialog.cpp \
    ../component/mytitlebar.cpp \
    framelessExtended/cursorposcalculator.cpp \
    framelessExtended/framelesshandle.cpp \
    framelessExtended/widgethandlerealize.cpp \
    maincontroller.cpp \
    singleProcessNet/devices.cpp \
    singleProcessNet/scanthread.cpp \
    singleProcessNet/packet.cpp \
    singleProcessNet/process.cpp \
    singleProcessNet/connection.cpp \
    singleProcessNet/conninode.cpp \
    singleProcessNet/inode2prog.cpp \
    singleProcessNet/refreshthread.cpp \
    singleProcessNet/decpcap.c
#    newresourcesdialog.cpp

OTHER_FILES += \
    systemmonitor.json

#RESOURCES += \
#    img.qrc

RESOURCES += \
    res.qrc

TRANSLATIONS += \
    translation/ukui-system-monitor_bo.ts \
    translation/ukui-system-monitor_de.ts \
    translation/ukui-system-monitor_es.ts \
    translation/ukui-system-monitor_fr.ts \
    translation/ukui-system-monitor_ru.ts \
    translation/ukui-system-monitor_tr.ts \
    translation/ukui-system-monitor_zh_CN.ts
