#include "mainwindow.h"
#include "configmanager.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("MarkEdit");
    QApplication::setApplicationName("MarkEdit");
    QApplication::setApplicationVersion("1.0.0");

    // Ensure UTF-8 handling for source / preview text.
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // Load persisted configuration (font, css, recent files, ...).
    ConfigManager::instance()->loadConfig();

    MainWindow window;
    window.show();

    return app.exec();
}
