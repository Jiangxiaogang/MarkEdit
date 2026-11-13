#include "stylesheetloader.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

namespace
{
    // Reads a UTF-8 text resource. Returns an empty string when unavailable.
    QString readUtf8(const QString &path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();

        QTextStream in(&file);
        in.setCodec("UTF-8");
        return in.readAll();
    }
} // namespace

QString StyleSheetLoader::defaultStyle()
{
    // The resource is immutable, so read it only once.
    static const QString css = readUtf8(QLatin1String(":/styles/default.css"));
    return css;
}

QString StyleSheetLoader::load(const QString &filePath, QString *errorMessage)
{
    // Empty path means the built-in style: not an error.
    if (filePath.isEmpty())
    {
        if (errorMessage)
            errorMessage->clear();
        return defaultStyle();
    }

    QFile file(filePath);
    if (!file.exists())
    {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate(
                "StyleSheetLoader", "File does not exist: %1").arg(filePath);
        return defaultStyle();
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate(
                "StyleSheetLoader", "Cannot open file: %1").arg(filePath);
        return defaultStyle();
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString css = in.readAll();
    file.close();

    QString error;
    if (!validate(css, &error))
    {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate(
                "StyleSheetLoader", "Invalid CSS (%1): %2").arg(error, filePath);
        return defaultStyle();
    }

    if (errorMessage)
        errorMessage->clear();
    return css;
}

bool StyleSheetLoader::validate(const QString &css, QString *errorMessage)
{
    int depth = 0;
    int line = 1;
    bool inComment = false;
    QChar quote;

    for (int i = 0; i < css.size(); ++i)
    {
        const QChar c = css.at(i);

        if (c == QLatin1Char('\n'))
        {
            ++line;
            continue;
        }

        if (inComment)
        {
            if (c == QLatin1Char('*') && i + 1 < css.size()
                    && css.at(i + 1) == QLatin1Char('/'))
            {
                inComment = false;
                ++i;
            }
            continue;
        }

        if (!quote.isNull())
        {
            if (c == QLatin1Char('\\'))
                ++i; // skip the escaped character
            else if (c == quote)
                quote = QChar();
            continue;
        }

        if (c == QLatin1Char('/') && i + 1 < css.size()
                && css.at(i + 1) == QLatin1Char('*'))
        {
            inComment = true;
            ++i;
            continue;
        }

        if (c == QLatin1Char('"') || c == QLatin1Char('\''))
        {
            quote = c;
            continue;
        }

        if (c == QLatin1Char('{'))
        {
            ++depth;
        }
        else if (c == QLatin1Char('}'))
        {
            --depth;
            if (depth < 0)
            {
                if (errorMessage)
                    *errorMessage = QCoreApplication::translate(
                        "StyleSheetLoader", "unexpected '}' at line %1").arg(line);
                return false;
            }
        }
    }

    if (depth != 0)
    {
        if (errorMessage)
            *errorMessage = QCoreApplication::translate(
                "StyleSheetLoader", "unclosed '{' at end of file");
        return false;
    }

    if (errorMessage)
        errorMessage->clear();
    return true;
}
