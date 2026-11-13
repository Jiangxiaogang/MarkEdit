#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>
#include <QList>

struct ParserOption
{
    QString key;       // config key under the "parser" group (also the cmark-gfm extension name)
    QString label;     // menu text shown to the user
    bool defaultOn;    // default value when not present in config
};

// Stateless wrapper around cmark-gfm. All parsing state is local to parse(),
// so the class is used through static methods instead of an instance.
class MarkdownParser
{
public:
    // Converts Markdown to HTML using the parser options from ConfigManager.
    static QString parse(const QString &markdown);

    // The full list of render options exposed in the UI.
    static QList<ParserOption> parserOptions();

private:
    MarkdownParser();
};

#endif // MARKDOWNPARSER_H
