QT += core gui webkit

CONFIG -= c++11

# Qt 4.8 compatibility
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x040800

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    editor/codeeditor.cpp \
    preview/previewwidget.cpp \
    parser/markdownparser.cpp \
    config/configmanager.cpp

HEADERS += \
    mainwindow.h \
    editor/codeeditor.h \
    preview/previewwidget.h \
    parser/markdownparser.h \
    config/configmanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
