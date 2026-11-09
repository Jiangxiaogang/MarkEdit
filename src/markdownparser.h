#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QObject>
#include <QString>
#include <QList>

struct ParserOption
{
    QString key;       // config key under the "parser" group (also the cmark-gfm extension name)
    QString label;     // menu text shown to the user
    bool defaultOn;    // default value when not present in config
};

class MarkdownParser : public QObject
{
    Q_OBJECT
public:
    explicit MarkdownParser(QObject *parent = 0);
    QString parse(const QString &markdown) const;

    /**
     * @brief The full list of render options exposed in the UI.
     */
    static QList<ParserOption> parserOptions();
};

#endif // MARKDOWNPARSER_H
