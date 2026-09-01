#include "stylesheetloader.h"

#include <QFile>
#include <QTextStream>

StyleSheetLoader::StyleSheetLoader(QObject *parent)
    : QObject(parent)
{
}

QString StyleSheetLoader::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists())
    {
        emit cssLoadFailed(tr("File does not exist: %1").arg(filePath));
        return getDefaultCSS();
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit cssLoadFailed(tr("Cannot open file: %1").arg(filePath));
        return getDefaultCSS();
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString css = in.readAll();
    file.close();

    if (!validateCSS(css))
    {
        emit cssLoadFailed(tr("Invalid CSS (unbalanced braces): %1").arg(filePath));
        return getDefaultCSS();
    }

    emit cssLoaded(css);
    return css;
}

bool StyleSheetLoader::validateCSS(const QString &css)
{
    int depth = 0;
    for (QChar c : css)
    {
        if (c == '{')
            ++depth;
        else if (c == '}')
        {
            --depth;
            if (depth < 0)
                return false;
        }
    }
    return depth == 0;
}

QString StyleSheetLoader::getDefaultCSS()
{
    return "";
}
