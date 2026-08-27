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
