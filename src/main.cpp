#include "mainwindow.h"
#include "configmanager.h"

#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLocale>
#include <QDir>
#include <QResource>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("MarkEdit");
    QApplication::setApplicationName("MarkEdit");
    QApplication::setApplicationVersion("1.0.0");

    // 加载应用程序样式表以修复菜单栏文字居中问题
    QFile styleFile(":/styles/app.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    }

    // Ensure UTF-8 handling for source / preview text.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // Load Chinese (Simplified) translation for UI from embedded resources
    QTranslator translator;
    if (translator.load(QLocale(QLocale::Chinese, QLocale::China), "markedit", "_", ":/translations")) {
        app.installTranslator(&translator);
    }

    // Load persisted configuration (font, css, recent files, ...).
    ConfigManager::instance()->loadConfig();

    MainWindow window;
    window.show();

    return app.exec();
}
