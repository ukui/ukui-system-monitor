QT -= gui

TEMPLATE = lib

CONFIG += staticlib link_pkgconfig \
            c++11

DEFINES += 

SOURCES += \
    process.cpp \
    packet.cpp \
    nethogs.cpp \
    libnethogs.cpp \
    inode2prog.cpp \
    devices.cpp \
    conninode.cpp \
    connection.cpp

HEADERS += \
    process.h \
    packet.h \
    nethogs.h \
    libnethogs.h \
    inode2prog.h \
    devices.h \
    conninode.h \
    connection.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
