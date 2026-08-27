#include "stylesheetloader.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

StyleSheetLoader::StyleSheetLoader(QObject *parent)
    : QObject(parent)
{
}

QString StyleSheetLoader::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        emit cssLoadFailed(tr("File does not exist: %1").arg(filePath));
        return getDefaultCSS();
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit cssLoadFailed(tr("Cannot open file: %1").arg(filePath));
        return getDefaultCSS();
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString css = in.readAll();
    file.close();

    if (!validateCSS(css)) {
        emit cssLoadFailed(tr("Invalid CSS (unbalanced braces): %1").arg(filePath));
        return getDefaultCSS();
    }

    emit cssLoaded(css);
    return css;
}

bool StyleSheetLoader::validateCSS(const QString &css)
{
    int depth = 0;
    for (QChar c : css) {
        if (c == '{')
            ++depth;
        else if (c == '}') {
            --depth;
            if (depth < 0)
                return false;
        }
    }
    return depth == 0;
}

QString StyleSheetLoader::getDefaultCSS()
{
    return
        "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', "
        "Helvetica, Arial, sans-serif; font-size: 16px; line-height: 1.6; "
        "color: #333; max-width: 900px; margin: 0 auto; padding: 20px; "
        "background-color: #fff; }\n"
        "h1, h2, h3, h4, h5, h6 { margin-top: 24px; margin-bottom: 16px; "
        "font-weight: 600; line-height: 1.25; }\n"
        "h1 { font-size: 2em; border-bottom: 1px solid #eaecef; padding-bottom: .3em; }\n"
        "h2 { font-size: 1.5em; border-bottom: 1px solid #eaecef; padding-bottom: .3em; }\n"
        "a { color: #0366d6; text-decoration: none; }\n"
        "code { padding: .2em .4em; font-size: 85%; background-color: rgba(27,31,35,.05); "
        "border-radius: 3px; font-family: Consolas, monospace; }\n"
        "pre { padding: 16px; overflow: auto; font-size: 85%; background-color: #f6f8fa; "
        "border-radius: 3px; }\n"
        "pre code { background: transparent; padding: 0; }\n"
        "blockquote { padding: 0 1em; color: #6a737d; border-left: .25em solid #dfe2e5; margin: 0; }\n"
        "hr { height: .25em; padding: 0; margin: 24px 0; background-color: #e1e4e8; border: 0; }\n"
        "del { text-decoration: line-through; }\n"
        "table { border-collapse: collapse; width: 100%; margin: 16px 0; "
        "display: block; overflow-x: auto; }\n"
        "table th, table td { border: 1px solid #dfe2e5; padding: 6px 13px; }\n"
        "table tr:nth-child(2n) { background-color: #f6f8fa; }\n"
        "table th { font-weight: 600; }\n";
}

QString StyleSheetLoader::getDarkCSS()
{
    return
        "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', "
        "Helvetica, Arial, sans-serif; font-size: 16px; line-height: 1.6; "
        "color: #c9d1d9; max-width: 900px; margin: 0 auto; padding: 20px; "
        "background-color: #0d1117; }\n"
        "h1, h2, h3, h4, h5, h6 { color: #f0f6fc; margin-top: 24px; margin-bottom: 16px; "
        "font-weight: 600; }\n"
        "h1, h2 { border-bottom: 1px solid #21262d; padding-bottom: .3em; }\n"
        "a { color: #58a6ff; text-decoration: none; }\n"
        "code { padding: .2em .4em; font-size: 85%; background-color: rgba(110,118,129,.4); "
        "border-radius: 3px; font-family: Consolas, monospace; }\n"
        "pre { padding: 16px; overflow: auto; font-size: 85%; background-color: #161b22; "
        "border-radius: 3px; }\n"
        "pre code { background: transparent; padding: 0; }\n"
        "blockquote { padding: 0 1em; color: #8b949e; border-left: .25em solid #30363d; margin: 0; }\n"
        "hr { height: .25em; padding: 0; margin: 24px 0; background-color: #30363d; border: 0; }\n"
        "del { text-decoration: line-through; }\n"
        "table { border-collapse: collapse; width: 100%; margin: 16px 0; "
        "display: block; overflow-x: auto; }\n"
        "table th, table td { border: 1px solid #30363d; padding: 6px 13px; }\n"
        "table tr:nth-child(2n) { background-color: #161b22; }\n"
        "table th { font-weight: 600; }\n";
}
