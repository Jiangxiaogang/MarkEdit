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
    return  "h1, h2 {border-bottom: 1px solid #eaecef; padding-bottom: 0.2em;}"
            "code {background-color: #eaecef; padding: 0.1em 0.4em;}"
            "pre {border: 1px solid #dfe2e5;background-color: #f6f8fa;padding: 16px;overflow: auto;}"
            "pre code {background: none;padding: 0;}"
            "blockquote {border-left: 4px solid #dfe2e5;color: #6a737d; padding: 0 1em;margin: 0;}"
            "ul { line-height: 1.5;}"
            "table {border-collapse: collapse;}"
            "table th {background-color: #f0f0f0;}"
            "table th, table td {border: 1px solid #dfe2e5;padding: 6px 6px;}";
}
