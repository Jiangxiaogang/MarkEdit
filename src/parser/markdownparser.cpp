#include "markdownparser.h"

MarkdownParser::MarkdownParser(QObject *parent)
    : QObject(parent)
{
}

QString MarkdownParser::parse(const QString &markdown)
{
    QString html = markdown;
    
    // 1. 解析代码块 (必须在其他解析之前，避免代码块内的标记被解析)
    html = parseCodeBlocks(html);
    
    // 2. 解析图片 (在链接之前，因为图片语法类似链接)
    html = parseImages(html);
    
    // 3. 解析链接
    html = parseLinks(html);
    
    // 4. 解析粗体 (在斜体之前)
    html = parseBold(html);
    
    // 5. 解析斜体
    html = parseItalic(html);
    
    // 6. 解析删除线
    html = parseStrikethrough(html);
    
    // 7. 解析行内代码
    html = parseInlineCode(html);
    
    // 8. 解析标题
    html = parseHeaders(html);
    
    // 9. 解析列表
    html = parseLists(html);
    
    // 10. 解析引用
    html = parseBlockquotes(html);
    
    // 11. 解析水平线
    html = parseHorizontalRules(html);
    
    // 12. 解析段落
    html = parseParagraphs(html);
    
    return html;
}

QString MarkdownParser::parseHeaders(const QString &text)
{
    QString result = text;
    
    // H6 到 H1 (必须从大到小，避免匹配冲突)
    result.replace(QRegularExpression("^######\\s+(.*)$", QRegularExpression::MultilineOption), "<h6>\\1</h6>");
    result.replace(QRegularExpression("^#####\\s+(.*)$", QRegularExpression::MultilineOption), "<h5>\\1</h5>");
    result.replace(QRegularExpression("^####\\s+(.*)$", QRegularExpression::MultilineOption), "<h4>\\1</h4>");
    result.replace(QRegularExpression("^###\\s+(.*)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    result.replace(QRegularExpression("^##\\s+(.*)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    result.replace(QRegularExpression("^#\\s+(.*)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");
    
    return result;
}

QString MarkdownParser::parseBold(const QString &text)
{
    QString result = text;
    // **bold** or __bold__
    result.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
    result.replace(QRegularExpression("__(.+?)__"), "<strong>\\1</strong>");
    return result;
}

QString MarkdownParser::parseItalic(const QString &text)
{
    QString result = text;
    // *italic* or _italic_
    result.replace(QRegularExpression("\\*(.+?)\\*"), "<em>\\1</em>");
    result.replace(QRegularExpression("_(.+?)_"), "<em>\\1</em>");
    return result;
}

QString MarkdownParser::parseStrikethrough(const QString &text)
{
    QString result = text;
    // ~~strikethrough~~
    result.replace(QRegularExpression("~~(.+?)~~"), "<del>\\1</del>");
    return result;
}

QString MarkdownParser::parseInlineCode(const QString &text)
{
    QString result = text;
    // `code`
    result.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    return result;
}

QString MarkdownParser::parseCodeBlocks(const QString &text)
{
    QString result = text;
    
    // ```code``` (支持带语言标识的代码块)
    QRegularExpression codeBlockRegex("```(\\w*)\\n([\\s\\S]*?)```", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = codeBlockRegex.globalMatch(result);
    
    QStringList matches;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString lang = match.captured(1);
        QString code = match.captured(2).toHtmlEscaped();
        
        QString replacement;
        if (!lang.isEmpty()) {
            replacement = QString("<pre><code class=\"language-%1\">%2</code></pre>").arg(lang).arg(code);
        } else {
            replacement = QString("<pre><code>%1</code></pre>").arg(code);
        }
        
        matches << match.captured(0) << replacement;
    }
    
    for (int i = 0; i < matches.size(); i += 2) {
        result.replace(matches[i], matches[i + 1]);
    }
    
    return result;
}

QString MarkdownParser::parseLinks(const QString &text)
{
    QString result = text;
    // [text](url)
    result.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    return result;
}

QString MarkdownParser::parseImages(const QString &text)
{
    QString result = text;
    // ![alt](url)
    result.replace(QRegularExpression("!\\[([^\\]]*)\\]\\(([^)]+)\\)"), "<img src=\"\\2\" alt=\"\\1\"/>");
    return result;
}

QString MarkdownParser::parseLists(const QString &text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList outputLines;
    
    bool inBulletList = false;
    bool inNumberedList = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        // 检查无序列表
        QRegularExpression bulletRegex("^\\s*[-*+]\\s+(.*)$");
        QRegularExpressionMatch bulletMatch = bulletRegex.match(line);
        
        // 检查有序列表
        QRegularExpression numberedRegex("^\\s*\\d+\\.\\s+(.*)$");
        QRegularExpressionMatch numberedMatch = numberedRegex.match(line);
        
        if (bulletMatch.hasMatch()) {
            if (!inBulletList) {
                outputLines << "<ul>";
                inBulletList = true;
                inNumberedList = false;
            }
            outputLines << QString("<li>%1</li>").arg(bulletMatch.captured(1));
        } else if (numberedMatch.hasMatch()) {
            if (!inNumberedList) {
                if (inBulletList) {
                    outputLines << "</ul>";
                    inBulletList = false;
                }
                outputLines << "<ol>";
                inNumberedList = true;
            }
            outputLines << QString("<li>%1</li>").arg(numberedMatch.captured(1));
        } else {
            if (inBulletList) {
                outputLines << "</ul>";
                inBulletList = false;
            }
            if (inNumberedList) {
                outputLines << "</ol>";
                inNumberedList = false;
            }
            outputLines << line;
        }
    }
    
    // 关闭未关闭的列表
    if (inBulletList) {
        outputLines << "</ul>";
    }
    if (inNumberedList) {
        outputLines << "</ol>";
    }
    
    return outputLines.join('\n');
}

QString MarkdownParser::parseBlockquotes(const QString &text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList outputLines;
    
    bool inBlockquote = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        QRegularExpression quoteRegex("^&gt;\\s*(.*)$");
        QRegularExpressionMatch match = quoteRegex.match(line);
        
        if (match.hasMatch()) {
            if (!inBlockquote) {
                outputLines << "<blockquote>";
                inBlockquote = true;
            }
            outputLines << match.captured(1);
        } else {
            if (inBlockquote) {
                outputLines << "</blockquote>";
                inBlockquote = false;
            }
            outputLines << line;
        }
    }
    
    if (inBlockquote) {
        outputLines << "</blockquote>";
    }
    
    return outputLines.join('\n');
}

QString MarkdownParser::parseHorizontalRules(const QString &text)
{
    QString result = text;
    // ---, ***, ___
    result.replace(QRegularExpression("^---+$", QRegularExpression::MultilineOption), "<hr/>");
    result.replace(QRegularExpression("^\\*\\*\\*+$", QRegularExpression::MultilineOption), "<hr/>");
    result.replace(QRegularExpression("^___+$", QRegularExpression::MultilineOption), "<hr/>");
    return result;
}

QString MarkdownParser::parseParagraphs(const QString &text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList outputLines;
    
    QStringList paragraphLines;
    
    auto flushParagraph = [&]() {
        if (!paragraphLines.isEmpty()) {
            QString paraText = paragraphLines.join(" ");
            if (!paraText.trimmed().isEmpty() && 
                !paraText.trimmed().startsWith('<')) {
                outputLines << QString("<p>%1</p>").arg(paraText);
            } else {
                outputLines << paragraphLines;
            }
            paragraphLines.clear();
        }
    };
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        
        // 如果是 HTML 标签或空行，刷新段落
        if (trimmed.isEmpty() || trimmed.startsWith('<')) {
            flushParagraph();
            outputLines << line;
        } else {
            paragraphLines << trimmed;
        }
    }
    
    flushParagraph();
    
    return outputLines.join('\n');
}
