#
QT       += core gui widgets printsupport webkit
TARGET    = MarkEdit
TEMPLATE  = app
CONFIG   += c++11
QMAKE_CFLAGS += -std=c11        # C 语言标准
QMAKE_CXXFLAGS += -std=c++11    # C++ 语言标准

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
RESOURCES +=

# Translations
TRANSLATIONS += translations/markedit_zh_CN.ts

# Enable warning-friendly build
QMAKE_CXXFLAGS_WARN_ON = -Wall
