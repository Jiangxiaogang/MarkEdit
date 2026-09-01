#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegExp>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit MarkdownHighlighter(QTextDocument *document = 0);

protected:
    void highlightBlock(const QString &text);

private:
    void applyRegex(const QString &text, const QRegExp &re, const QTextCharFormat &format);

    enum BlockState { Normal = 0, InCodeBlock = 1 };

    QTextCharFormat m_headingFmt;
    QTextCharFormat m_codeFmt;
    QTextCharFormat m_quoteFmt;
    QTextCharFormat m_listFmt;
    QTextCharFormat m_hrFmt;
    QTextCharFormat m_boldFmt;
    QTextCharFormat m_italicFmt;
    QTextCharFormat m_strikeFmt;
    QTextCharFormat m_linkFmt;
    QTextCharFormat m_tableFmt;
};

#endif // MARKDOWNHIGHLIGHTER_H
