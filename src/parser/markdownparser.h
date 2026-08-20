#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QRegExp>

class MarkdownParser : public QObject
{
    Q_OBJECT
    
public:
    explicit MarkdownParser(QObject *parent = nullptr);
    
    QString parse(const QString &markdown);
    
private:
    QString parseHeaders(const QString &text);
    QString parseBold(const QString &text);
    QString parseItalic(const QString &text);
    QString parseStrikethrough(const QString &text);
    QString parseInlineCode(const QString &text);
    QString parseCodeBlocks(const QString &text);
    QString parseLinks(const QString &text);
    QString parseImages(const QString &text);
    QString parseLists(const QString &text);
    QString parseBlockquotes(const QString &text);
    QString parseHorizontalRules(const QString &text);
    QString parseParagraphs(const QString &text);
    QString parseTables(const QString &text);
    QStringList parseTableRow(const QString &row, bool isHeader = false);
};

#endif // MARKDOWNPARSER_H
