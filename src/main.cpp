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

    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ConfigManager::instance()->loadConfig();

    // Open the first non-option argument as a file, if provided.
    QString filePath;
    const QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i)
    {
        const QString &a = args.at(i);
        if (a.startsWith('-'))
            continue;
        filePath = a;
        break;
    }

    MainWindow window(filePath);
    window.show();

    return app.exec();
}
