TARGET = RawKbdQt
TEMPLATE = app
QT += core gui widgets

SOURCES += main.cpp\
           mainwindow.cpp \
           nativefilter.cpp

HEADERS += mainwindow.h \
           nativefilter.h

linux {
    lessThan(QT_MAJOR_VERSION, 6) {
        QT += x11extras
    }
    CONFIG += link_pkgconfig
    PKGCONFIG += xcb
}

macx {
    HEADERS += maceventhelper.h
    OBJECTIVE_SOURCES += maceventhelper.mm
    LIBS += -framework Cocoa
}
