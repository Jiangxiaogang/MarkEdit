#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>

/**
 * @brief Lightweight Markdown syntax highlighter for the source editor.
 *
 * It only changes the *text colour* of recognised Markdown tokens. No font
 * family, weight, style or size is altered, so the editor keeps its normal
 * appearance (only colours are added). Multi-line fenced code blocks are
 * tracked via the highlighter's block state.
 */
class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit MarkdownHighlighter(QTextDocument *document = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    void applyRegex(const QString &text, const QRegularExpression &re,
                    const QTextCharFormat &format);

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
