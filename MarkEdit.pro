#-------------------------------------------------
# MarkEdit - A cross-platform Markdown editor
# Qt 5.4 / qmake project file
#-------------------------------------------------

QT       += core gui widgets printsupport webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MarkEdit
TEMPLATE = app

# C++11 (Qt 5.4 compatible)
CONFIG += c++11

# Source files
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/codeeditor.cpp \
    src/linenumberarea.cpp \
    src/markdownhighlighter.cpp \
    src/previewwidget.cpp \
    src/stylesheetloader.cpp \
    src/markdownparser.cpp \
    src/configmanager.cpp \
    src/findreplacedialog.cpp \
    src/settingsdialog.cpp \
    src/insertdialog.cpp \
    src/aboutdialog.cpp

# Header files
HEADERS += \
    src/mainwindow.h \
    src/codeeditor.h \
    src/linenumberarea.h \
    src/markdownhighlighter.h \
    src/previewwidget.h \
    src/stylesheetloader.h \
    src/markdownparser.h \
    src/configmanager.h \
    src/findreplacedialog.h \
    src/settingsdialog.h \
    src/insertdialog.h \
    src/aboutdialog.h

# Resources (icons)
RESOURCES += resources/resources.qrc

# Translations
TRANSLATIONS += translations/markedit_zh_CN.ts

# Default CSS style shipped with the application
# (copied into the binary resources so it is always available)
# -> see resources/resources.qrc

# Install layout (optional)
# target.path = /usr/local/bin
# INSTALLS += target

# Enable warning-friendly build
QMAKE_CXXFLAGS_WARN_ON = -Wall
