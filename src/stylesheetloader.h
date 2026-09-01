#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QObject>
#include <QString>

class StyleSheetLoader : public QObject
{
    Q_OBJECT
public:
    explicit StyleSheetLoader(QObject *parent = 0);

    QString loadFromFile(const QString &filePath);
    static QString getDefaultCSS();
    bool validateCSS(const QString &css);

signals:
    void cssLoaded(const QString &css);
    void cssLoadFailed(const QString &error);
};

#endif // STYLESHEETLOADER_H
