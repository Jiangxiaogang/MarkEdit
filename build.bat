@echo off
set QTDIR=D:\IDE\QT\Qt5.4.2\5.4\mingw491_32
set QMAKESPEC=win32-g++
set PATH=%QTDIR%\bin;%QTDIR%\..\..\Tools\mingw491_32\bin;%PATH%;
qmake -r
mingw32-make
