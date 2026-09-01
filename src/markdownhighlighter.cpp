#include "markdownhighlighter.h"
#include "configmanager.h"

#include <QRegExp>
#include <QTextCharFormat>

namespace
{
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
    // Highlight colours are read from the configuration file (config.ini),
    // falling back to the hard-coded defaults below if not configured.
    ConfigManager *cfg = ConfigManager::instance();
    m_headingFmt = colorFormat(QColor(cfg->headingColor()));
    m_codeFmt    = colorFormat(QColor(cfg->codeColor()));
    m_quoteFmt   = colorFormat(QColor(cfg->quoteColor()));
    m_listFmt    = colorFormat(QColor(cfg->listColor()));
    m_hrFmt      = colorFormat(QColor(cfg->hrColor()));
    m_boldFmt    = colorFormat(QColor(cfg->boldColor()));
    m_italicFmt  = colorFormat(QColor(cfg->italicColor()));
    m_strikeFmt  = colorFormat(QColor(cfg->strikeColor()));
    m_linkFmt    = colorFormat(QColor(cfg->linkColor()));
    m_tableFmt   = colorFormat(QColor(cfg->tableColor()));
}

void MarkdownHighlighter::applyRegex(const QString &text,
                                     const QRegExp &re,
                                     const QTextCharFormat &format)
{
    int pos = 0;
    while ((pos = re.indexIn(text, pos)) != -1)
    {
        int len = re.matchedLength();
        setFormat(pos, len, format);
        pos += len;
        if (len == 0) ++pos; // prevent infinite loop on zero-length match
    }
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    int state = previousBlockState();
    if (state < 0)
        state = Normal;

    QRegExp fenceRe("^\\s*(`{3,}|~{3,})(.*)$");
    QRegExp closeRe("^\\s*(`{3,}|~{3,})\\s*$");

    // ---- Inside a fenced code block: colour the entire line as code ----
    if (state == InCodeBlock)
    {
        setFormat(0, text.length(), m_codeFmt);
        setCurrentBlockState(closeRe.exactMatch(text) ? Normal : InCodeBlock);
        return;
    }

    // ---- Opening fence line ----
    if (fenceRe.exactMatch(text))
    {
        setFormat(0, text.length(), m_codeFmt);
        setCurrentBlockState(InCodeBlock);
        return;
    }

    // ---- Heading (whole line) ----
    if (QRegExp("^(#{1,6})\\s+").exactMatch(text))
    {
        setFormat(0, text.length(), m_headingFmt);
        setCurrentBlockState(Normal);
        return;
    }

    // ---- Horizontal rule (whole line) ----
    if (QRegExp("^(\\s*([-*_])\\s*){3,}$").exactMatch(text))
    {
        setFormat(0, text.length(), m_hrFmt);
        setCurrentBlockState(Normal);
        return;
    }

    // ---- Block quote marker (just the leading '>') ----
    QRegExp bqRe("^(\\s*>)");
    if (bqRe.indexIn(text) != -1)
        setFormat(bqRe.pos(1), bqRe.cap(1).length(), m_quoteFmt);

    // ---- List marker (just the bullet / number) ----
    QRegExp lmRe("^(\\s*)([-*+]|\\d+\\.)(\\s+)");
    if (lmRe.indexIn(text) != -1)
        setFormat(lmRe.pos(2), lmRe.cap(2).length(), m_listFmt);

    // ---- Inline emphasis / structure ----
    // Bold first so ** and __ are consumed before single * and _.
    applyRegex(text, QRegExp("\\*\\*(.+?)\\*\\*"), m_boldFmt);
    applyRegex(text, QRegExp("(?<!\\w)__(?!\\s)(.+?)(?<!\\s)__(?!\\w)"), m_boldFmt);
    // Italic: prevent matching the '*' that belongs to a '**' pair.
    applyRegex(text, QRegExp("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)"), m_italicFmt);
    applyRegex(text, QRegExp("(?<!\\w)_(?!\\s)(.+?)(?<!\\s)_(?!\\w)"), m_italicFmt);
    applyRegex(text, QRegExp("~~(.+?)~~"), m_strikeFmt);
    applyRegex(text, QRegExp("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)"), m_linkFmt);
    applyRegex(text, QRegExp("\\[([^\\]]*)\\]\\(([^\\)]+)\\)"), m_linkFmt);
    applyRegex(text, QRegExp("\\|"), m_tableFmt);
    // Inline code is applied last so it overrides any emphasis markers that
    // happen to appear inside backticks.
    applyRegex(text, QRegExp("`[^`\\n]+`"), m_codeFmt);

    setCurrentBlockState(Normal);
}
