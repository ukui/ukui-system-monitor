TEMPLATE = subdirs

#CONFIG += ordered

SUBDIRS = \
    procnet-monitor \
    src \

src.depends = procnet-monitor
