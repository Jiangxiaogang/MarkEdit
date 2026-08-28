#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QObject>
#include <QString>

/**
 * @brief Converts Markdown text into HTML.
 *
 * A self-contained, line-based Markdown parser. It supports the commonly used
 * subset of Markdown:
 *   - ATX headings (# .. ######)
 *   - bold (**text** / __text__), italic (*text* / _text_), strikethrough (~~text~~)
 *   - inline code (`code`) and fenced code blocks (```)
 *   - unordered (-, *, +) and ordered (1.) lists
 *   - block quotes (> text)
 *   - links [text](url) and images ![alt](url)
 *   - horizontal rules (---, ***, ___)
 *   - paragraphs
 *   - tables (GitHub Flavored Markdown pipe tables with column alignment)
 *
 * The parser deliberately favours robustness over 100% CommonMark compliance,
 * which is sufficient for a live preview widget.
 */
class MarkdownParser : public QObject
{
    Q_OBJECT
public:
    explicit MarkdownParser(QObject *parent = 0);

    /**
     * @brief Parse @p markdown and return an HTML fragment (body content).
     */
    QString parse(const QString &markdown) const;

private:
    QString escapeHtml(const QString &text) const;
    QString parseInline(const QString &text) const;

    // Block-level helpers (return HTML and advance the line cursor)
    QString parseBlock(const QStringList &lines, int &i) const;

    QString parseCodeBlock(const QStringList &lines, int &i) const;
    QString parseList(const QStringList &lines, int &i) const;
    QString parseBlockQuote(const QStringList &lines, int &i) const;
    QString parseHeading(const QString &line) const;
    QString parseHorizontalRule(const QString &line) const;
    QString parseParagraph(const QStringList &lines, int &i) const;

    // Tables (GitHub Flavored Markdown pipe tables)
    QString parseTable(const QStringList &lines, int &i) const;
    QStringList splitTableRow(const QString &line) const;
    bool isTableDelimiter(const QString &line, QStringList &aligns) const;
};

#endif // MARKDOWNPARSER_H
