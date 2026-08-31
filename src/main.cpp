#include "mainwindow.h"
#include "configmanager.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    printf("111\n");
    QApplication app(argc, argv);
    QApplication::setOrganizationName("MarkEdit");
    QApplication::setApplicationName("MarkEdit");
    QApplication::setApplicationVersion("1.0.0");

    // Ensure UTF-8 handling for source / preview text.
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // Load persisted configuration
    ConfigManager::instance()->loadConfig();

    MainWindow window;
    window.show();

    return app.exec();
}
