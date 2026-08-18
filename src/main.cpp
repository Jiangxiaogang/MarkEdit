#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    QCoreApplication::setOrganizationName("MarkdownEditor");
    QCoreApplication::setApplicationName("Markdown Editor");
    QCoreApplication::setApplicationVersion("1.0");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
