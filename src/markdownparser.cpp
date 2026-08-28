#include "markdownparser.h"

#include <QRegExp>
#include <QStringList>

MarkdownParser::MarkdownParser(QObject *parent)
    : QObject(parent)
{
}

QString MarkdownParser::escapeHtml(const QString &text) const
{
    QString out = text;
    out.replace("&", "&amp;");
    out.replace("<", "&lt;");
    out.replace(">", "&gt;");
    return out;
}

QString MarkdownParser::parseInline(const QString &text) const
{
    QString out = escapeHtml(text);

    // Extract inline code spans first so their content is not touched by the
    // emphasis / link replacements below. A private placeholder is used.
    QStringList codes;
    int pos = 0;
    while (true) {
        int start = out.indexOf('`', pos);
        if (start < 0)
            break;
        int end = out.indexOf('`', start + 1);
        if (end < 0)
            break;
        codes.append(out.mid(start + 1, end - start - 1));
        QString placeholder = QString("\x01C%1\x01").arg(codes.size() - 1);
        out.replace(start, end - start + 1, placeholder);
        pos = start + placeholder.length();
    }

    // Images: ![alt](url)  (must be processed before links)
    QRegExp imgRe("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)");
    pos = 0;
    while ((pos = imgRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<img src=\"%2\" alt=\"%1\">")
            .arg(imgRe.cap(1)).arg(imgRe.cap(2));
        out.replace(pos, imgRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Links: [text](url)
    QRegExp linkRe("\\[([^\\]]*)\\]\\(([^\\)]+)\\)");
    pos = 0;
    while ((pos = linkRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<a href=\"%2\">%1</a>")
            .arg(linkRe.cap(1)).arg(linkRe.cap(2));
        out.replace(pos, linkRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Bold: **text**
    QRegExp boldStarRe("\\*\\*(.+?)\\*\\*");
    pos = 0;
    while ((pos = boldStarRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<strong>%1</strong>").arg(boldStarRe.cap(1));
        out.replace(pos, boldStarRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Bold: __text__  (word-boundary aware so intra-word underscores stay literal)
    QRegExp boldUnderscoreRe("(?<!\\w)__(?!\\s)(.*?)(?<!\\s)__(?!\\w)");
    pos = 0;
    while ((pos = boldUnderscoreRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<strong>%1</strong>").arg(boldUnderscoreRe.cap(1));
        out.replace(pos, boldUnderscoreRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Italic: *text*
    QRegExp italicStarRe("\\*(.+?)\\*");
    pos = 0;
    while ((pos = italicStarRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<em>%1</em>").arg(italicStarRe.cap(1));
        out.replace(pos, italicStarRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Italic: _text_  (word-boundary aware so intra-word underscores stay literal)
    QRegExp italicUnderscoreRe("(?<!\\w)_(?!\\s)(.+?)(?<!\\s)_(?!\\w)");
    pos = 0;
    while ((pos = italicUnderscoreRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<em>%1</em>").arg(italicUnderscoreRe.cap(1));
        out.replace(pos, italicUnderscoreRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Strikethrough: ~~text~~
    QRegExp strikeRe("~~(.+?)~~");
    pos = 0;
    while ((pos = strikeRe.indexIn(out, pos)) != -1) {
        QString replacement = QString("<del>%1</del>").arg(strikeRe.cap(1));
        out.replace(pos, strikeRe.matchedLength(), replacement);
        pos += replacement.length();
    }

    // Restore inline code spans.
    for (int i = 0; i < codes.size(); ++i)
        out.replace(QString("\x01C%1\x01").arg(i), "<code>" + codes.at(i) + "</code>");

    return out;
}

QString MarkdownParser::parseHeading(const QString &line) const
{
    QRegExp re("^(#{1,6})\\s+(.*)$");
    if (!re.exactMatch(line))
        return QString();
    int level = re.cap(1).length();
    return QString("<h%1>%2</h%1>\n").arg(level).arg(parseInline(re.cap(2).trimmed()));
}

QString MarkdownParser::parseHorizontalRule(const QString &line) const
{
    QRegExp re("^(\\s*([-*_])\\s*){3,}$");
    if (re.exactMatch(line))
        return "<hr>\n";
    return QString();
}

QString MarkdownParser::parseCodeBlock(const QStringList &lines, int &i) const
{
    // lines[i] is a fence opening: ``` or ~~~
    QString fence = lines[i];
    QString lang;
    QRegExp fenceRe("^(\\s*)(`{3,}|~{3,})\\s*(.*)$");
    if (fenceRe.exactMatch(fence))
        lang = fenceRe.cap(3).trimmed();

    QStringList code;
    ++i;
    QRegExp closeRe("^(\\s*)(`{3,}|~{3,})\\s*$");
    while (i < lines.size() && !closeRe.exactMatch(lines[i])) {
        code.append(lines[i]);
        ++i;
    }
    // consume closing fence if present
    if (i < lines.size())
        ++i;

    QString cls = lang.isEmpty() ? QString() : QString(" class=\"language-%1\"").arg(lang);
    return QString("<pre><code%1>%2</code></pre>\n").arg(cls).arg(escapeHtml(code.join("\n")));
}

QString MarkdownParser::parseList(const QStringList &lines, int &i) const
{
    QString first = lines[i];
    QRegExp orderedRe("^\\s*\\d+\\.\\s+(.*)$");
    QRegExp unorderedRe("^\\s*([-*+])\\s+(.*)$");

    bool ordered = orderedRe.exactMatch(first);
    QString tag = ordered ? "ol" : "ul";

    QString html = QString("<%1>\n").arg(tag);
    while (i < lines.size()) {
        QString cur = lines[i];
        if (cur.trimmed().isEmpty())
            break;
        QRegExp &re = ordered ? orderedRe : unorderedRe;
        if (!re.exactMatch(cur))
            break;
        html += QString("  <li>%1</li>\n").arg(parseInline(re.cap(ordered ? 1 : 2).trimmed()));
        ++i;
    }
    html += QString("</%1>\n").arg(tag);
    return html;
}

QString MarkdownParser::parseBlockQuote(const QStringList &lines, int &i) const
{
    QStringList quote;
    QRegExp re("^\\s*>\\s?(.*)$");
    while (i < lines.size()) {
        if (!re.exactMatch(lines[i]))
            break;
        quote.append(re.cap(1));
        ++i;
    }
    return QString("<blockquote>\n%1</blockquote>\n").arg(parseInline(quote.join(" ")).replace("\n", " "));
}

QString MarkdownParser::parseParagraph(const QStringList &lines, int &i) const
{
    QStringList para;
    while (i < lines.size() && !lines[i].trimmed().isEmpty()) {
        // Stop if the next line starts a new block
        QString l = lines[i];
        if (parseHeading(l).isEmpty() == false) break;
        if (parseHorizontalRule(l).isEmpty() == false) break;
        if (QRegExp("^\\s*([-*+])\\s+").indexIn(l) != -1) break;
        if (QRegExp("^\\s*\\d+\\.\\s+").indexIn(l) != -1) break;
        if (QRegExp("^\\s*>\\s?").indexIn(l) != -1) break;
        if (QRegExp("^(\\s*)(`{3,}|~{3,})").indexIn(l) != -1) break;

        para.append(l);
        ++i;
    }
    return QString("<p>%1</p>\n").arg(parseInline(para.join(" ")));
}

QStringList MarkdownParser::splitTableRow(const QString &line) const
{
    QString l = line.trimmed();
    // Drop a single leading/trailing pipe if present.
    if (l.startsWith('|'))
        l = l.mid(1);
    if (l.endsWith('|'))
        l.chop(1);

    QStringList parts;
    QString cur;
    bool escaped = false;
    for (int k = 0; k < l.length(); ++k) {
        QChar c = l.at(k);
        if (escaped) {
            cur += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '|') {
            parts.append(cur.trimmed());
            cur.clear();
        } else {
            cur += c;
        }
    }
    parts.append(cur.trimmed());
    return parts;
}

bool MarkdownParser::isTableDelimiter(const QString &line, QStringList &aligns) const
{
    aligns.clear();
    if (!line.contains('|'))
        return false;

    QStringList cells = splitTableRow(line);
    if (cells.isEmpty())
        return false;

    QRegExp dashRe("^:?-+:?$");
    for (const QString &cell : cells) {
        QString c = cell.trimmed();
        if (!dashRe.exactMatch(c))
            return false;
        QString align;
        if (c.startsWith(':') && c.endsWith(':'))
            align = "center";
        else if (c.endsWith(':'))
            align = "right";
        else if (c.startsWith(':'))
            align = "left";
        aligns.append(align);
    }
    return true;
}

QString MarkdownParser::parseTable(const QStringList &lines, int &i) const
{
    QString headerLine = lines[i];
    if (!headerLine.contains('|'))
        return QString();
    if (i + 1 >= lines.size())
        return QString();

    QStringList aligns;
    if (!isTableDelimiter(lines[i + 1], aligns))
        return QString();

    int cols = aligns.size();
    QStringList headers = splitTableRow(headerLine);
    // Normalise header to the column count defined by the delimiter row.
    while (headers.size() < cols)
        headers.append(QString());

    auto alignAttr = [&](const QString &a) -> QString {
        return a.isEmpty() ? QString() : QString(" style=\"text-align:%1\"").arg(a);
    };

    QString html = "<table>\n<thead>\n<tr>";
    for (int c = 0; c < cols; ++c)
        html += QString("<th%1>%2</th>").arg(alignAttr(aligns.at(c)))
                .arg(parseInline(headers.value(c)));
    html += "</tr>\n</thead>\n<tbody>\n";

    // Consume header + delimiter rows.
    i += 2;
    while (i < lines.size()) {
        QString l = lines[i];
        if (l.trimmed().isEmpty())
            break;
        if (parseHorizontalRule(l).isEmpty() == false)
            break;
        if (parseHeading(l).isEmpty() == false)
            break;
        if (!l.contains('|'))
            break;

        QStringList cells = splitTableRow(l);
        while (cells.size() < cols)
            cells.append(QString());
        if (cells.size() > cols) {
            // Overflow cells are merged into the last column.
            QString overflow = QStringList(cells.mid(cols - 1)).join(" | ");
            cells = cells.mid(0, cols - 1);
            cells.append(overflow);
        }

        html += "<tr>";
        for (int c = 0; c < cols; ++c)
            html += QString("<td%1>%2</td>").arg(alignAttr(aligns.at(c)))
                    .arg(parseInline(cells.value(c)));
        html += "</tr>\n";
        ++i;
    }
    html += "</tbody>\n</table>\n";
    return html;
}

QString MarkdownParser::parseBlock(const QStringList &lines, int &i) const
{
    QString line = lines[i];

    // Horizontal rule
    QString hr = parseHorizontalRule(line);
    if (!hr.isEmpty()) { ++i; return hr; }

    // Heading
    QString heading = parseHeading(line);
    if (!heading.isEmpty()) { ++i; return heading; }

    // Table (GitHub Flavored Markdown pipe table)
    QString table = parseTable(lines, i);
    if (!table.isEmpty())
        return table;

    // Fenced code block
    if (QRegExp("^(\\s*)(`{3,}|~{3,})").indexIn(line) != -1)
        return parseCodeBlock(lines, i);

    // Block quote
    if (QRegExp("^\\s*>\\s?").indexIn(line) != -1)
        return parseBlockQuote(lines, i);

    // Lists
    if (QRegExp("^\\s*([-*+])\\s+").indexIn(line) != -1 ||
        QRegExp("^\\s*\\d+\\.\\s+").indexIn(line) != -1)
        return parseList(lines, i);

    // Paragraph (falls through to blank handling)
    return parseParagraph(lines, i);
}

QString MarkdownParser::parse(const QString &markdown) const
{
    QStringList lines = markdown.split("\n");
    QString html;

    int i = 0;
    while (i < lines.size()) {
        if (lines[i].trimmed().isEmpty()) {
            ++i;
            continue;
        }
        html += parseBlock(lines, i);
    }

    return html;
}
