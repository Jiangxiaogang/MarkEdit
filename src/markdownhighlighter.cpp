#include "markdownhighlighter.h"

#include <QRegularExpression>
#include <QTextCharFormat>

namespace {

// Build a character format that only carries a foreground colour. This is what
// guarantees the highlighter never changes font family, weight, style or size.
QTextCharFormat colorFormat(const QColor &color)
{
    QTextCharFormat f;
    f.setForeground(color);
    return f;
}

} // namespace

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{
    // GitHub-inspired palette, tuned to stay readable on a light editor
    // background. Only the colour matters here.
    m_headingFmt = colorFormat(QColor("#1f6feb"));
    m_codeFmt    = colorFormat(QColor("#cf222e"));
    m_quoteFmt   = colorFormat(QColor("#57606a"));
    m_listFmt    = colorFormat(QColor("#e36209"));
    m_hrFmt      = colorFormat(QColor("#57606a"));
    m_boldFmt    = colorFormat(QColor("#8250df"));
    m_italicFmt  = colorFormat(QColor("#0c7b93"));
    m_strikeFmt  = colorFormat(QColor("#57606a"));
    m_linkFmt    = colorFormat(QColor("#0550ae"));
    m_tableFmt   = colorFormat(QColor("#57606a"));
}

void MarkdownHighlighter::applyRegex(const QString &text,
                                     const QRegularExpression &re,
                                     const QTextCharFormat &format)
{
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), format);
    }
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    int state = previousBlockState();
    if (state < 0)
        state = Normal;

    QRegularExpression fenceRe("^\\s*(`{3,}|~{3,})(.*)$");
    QRegularExpression closeRe("^\\s*(`{3,}|~{3,})\\s*$");

    // ---- Inside a fenced code block: colour the entire line as code ----
    if (state == InCodeBlock) {
        setFormat(0, text.length(), m_codeFmt);
        setCurrentBlockState(closeRe.match(text).hasMatch() ? Normal : InCodeBlock);
        return;
    }

    // ---- Opening fence line ----
    if (fenceRe.match(text).hasMatch()) {
        setFormat(0, text.length(), m_codeFmt);
        setCurrentBlockState(InCodeBlock);
        return;
    }

    // ---- Heading (whole line) ----
    if (QRegularExpression("^(#{1,6})\\s+").match(text).hasMatch()) {
        setFormat(0, text.length(), m_headingFmt);
        setCurrentBlockState(Normal);
        return;
    }

    // ---- Horizontal rule (whole line) ----
    if (QRegularExpression("^(\\s*([-*_])\\s*){3,}$").match(text).hasMatch()) {
        setFormat(0, text.length(), m_hrFmt);
        setCurrentBlockState(Normal);
        return;
    }

    // ---- Block quote marker (just the leading '>') ----
    QRegularExpressionMatch bq = QRegularExpression("^(\\s*>)").match(text);
    if (bq.hasMatch())
        setFormat(bq.capturedStart(1), bq.capturedLength(1), m_quoteFmt);

    // ---- List marker (just the bullet / number) ----
    QRegularExpressionMatch lm = QRegularExpression("^(\\s*)([-*+]|\\d+\\.)(\\s+)").match(text);
    if (lm.hasMatch())
        setFormat(lm.capturedStart(2), lm.capturedLength(2), m_listFmt);

    // ---- Inline emphasis / structure ----
    // Bold first so ** and __ are consumed before single * and _.
    applyRegex(text, QRegularExpression("\\*\\*(.+?)\\*\\*"), m_boldFmt);
    applyRegex(text, QRegularExpression("(?<!\\w)__(?!\\s)(.+?)(?<!\\s)__(?!\\w)"), m_boldFmt);
    // Italic: prevent matching the '*' that belongs to a '**' pair.
    applyRegex(text, QRegularExpression("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)"), m_italicFmt);
    applyRegex(text, QRegularExpression("(?<!\\w)_(?!\\s)(.+?)(?<!\\s)_(?!\\w)"), m_italicFmt);
    applyRegex(text, QRegularExpression("~~(.+?)~~"), m_strikeFmt);
    applyRegex(text, QRegularExpression("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)"), m_linkFmt);
    applyRegex(text, QRegularExpression("\\[([^\\]]*)\\]\\(([^\\)]+)\\)"), m_linkFmt);
    applyRegex(text, QRegularExpression("\\|"), m_tableFmt);
    // Inline code is applied last so it overrides any emphasis markers that
    // happen to appear inside backticks.
    applyRegex(text, QRegularExpression("`[^`\\n]+`"), m_codeFmt);

    setCurrentBlockState(Normal);
}
