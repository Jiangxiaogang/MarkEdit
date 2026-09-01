#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QObject>
#include <QString>
#include <QList>

/**
 * @brief Describes one configurable cmark-gfm syntax extension.
 *
 * Each entry becomes a checkable item under the "语法扩展" (Syntax Extensions)
 * submenu and is persisted through ConfigManager. When enabled, the matching
 * extension is attached to the parser via cmark_parser_attach_syntax_extension.
 */
struct ParserOption
{
    QString key;       // config key under the "parser" group
    QString label;     // menu text shown to the user
    QString name;      // cmark-gfm extension name (e.g. "table")
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
