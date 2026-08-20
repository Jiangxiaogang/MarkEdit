QT += core gui widgets webenginewidgets printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact features
# depend on your Qt version). This helps you to ensure that your code
# will work with future versions of Qt.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your compiler emit warnings if someone tries to
# compile your code with private APIs. This is useful if you want to
# ensure that your code will work with future versions of Qt even if
# they remove the private APIs you are using today.
# DEFINES += QT_STRICT_WARNINGS

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
