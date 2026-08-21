#include "markdownparser.h"
#include <QStringList>
#include <QRegExp>

// Qt 4.8 兼容：手动实现 HTML 转义
static QString htmlEscape(const QString &text)
{
    QString result = text;
    result.replace(QLatin1String("&"), QLatin1String("&amp;"));
    result.replace(QLatin1String("<"), QLatin1String("&lt;"));
    result.replace(QLatin1String(">"), QLatin1String("&gt;"));
    result.replace(QLatin1String("\""), QLatin1String("&quot;"));
    result.replace(QLatin1String("'"), QLatin1String("&#39;"));
    return result;
}

MarkdownParser::MarkdownParser(QObject *parent)
    : QObject(parent)
{
}

QString MarkdownParser::parse(const QString &markdown)
{
    QString html = markdown;
    
    // 1. 解析代码块 (必须在其他解析之前，避免代码块内的标记被解析)
    html = parseCodeBlocks(html);
    
    // 2. 解析表格 (必须在段落解析之前，因为表格包含多行)
    html = parseTables(html);
    
    // 3. 解析图片 (在链接之前，因为图片语法类似链接)
    html = parseImages(html);
    
    // 4. 解析链接
    html = parseLinks(html);
    
    // 5. 解析粗体 (在斜体之前)
    html = parseBold(html);
    
    // 6. 解析斜体
    html = parseItalic(html);
    
    // 7. 解析删除线
    html = parseStrikethrough(html);
    
    // 8. 解析行内代码
    html = parseInlineCode(html);
    
    // 9. 解析标题
    html = parseHeaders(html);
    
    // 10. 解析列表
    html = parseLists(html);
    
    // 11. 解析引用
    html = parseBlockquotes(html);
    
    // 12. 解析水平线
    html = parseHorizontalRules(html);
    
    // 13. 解析段落
    html = parseParagraphs(html);
    
    return html;
}

QString MarkdownParser::parseHeaders(const QString &text)
{
    QString result = text;
    
    // H6 到 H1 (必须从大到小，避免匹配冲突)
    result.replace(QRegExp("^######\\s+(.*)$"), "<h6>\\1</h6>");
    result.replace(QRegExp("^#####\\s+(.*)$"), "<h5>\\1</h5>");
    result.replace(QRegExp("^####\\s+(.*)$"), "<h4>\\1</h4>");
    result.replace(QRegExp("^###\\s+(.*)$"), "<h3>\\1</h3>");
    result.replace(QRegExp("^##\\s+(.*)$"), "<h2>\\1</h2>");
    result.replace(QRegExp("^#\\s+(.*)$"), "<h1>\\1</h1>");
    
    return result;
}

QString MarkdownParser::parseBold(const QString &text)
{
    QString result = text;
    // **bold** or __bold__
    result.replace(QRegExp("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
    result.replace(QRegExp("__(.+?)__"), "<strong>\\1</strong>");
    return result;
}

QString MarkdownParser::parseItalic(const QString &text)
{
    QString result = text;
    // *italic* or _italic_
    result.replace(QRegExp("\\*(.+?)\\*"), "<em>\\1</em>");
    result.replace(QRegExp("_(.+?)_"), "<em>\\1</em>");
    return result;
}

QString MarkdownParser::parseStrikethrough(const QString &text)
{
    QString result = text;
    // ~~strikethrough~~
    result.replace(QRegExp("~~(.+?)~~"), "<del>\\1</del>");
    return result;
}

QString MarkdownParser::parseInlineCode(const QString &text)
{
    QString result = text;
    // `code`
    result.replace(QRegExp("`([^`]+)`"), "<code>\\1</code>");
    return result;
}

QString MarkdownParser::parseCodeBlocks(const QString &text)
{
    QString result = text;
    
    // ```code``` (支持带语言标识的代码块)
    QRegExp codeBlockRegex("```(\\w*)\\n([\\s\\S]*?)```");
    
    int pos = 0;
    while ((pos = codeBlockRegex.indexIn(result, pos)) != -1) {
        QString lang = codeBlockRegex.cap(1);
        QString code = htmlEscape(codeBlockRegex.cap(2));
        
        QString replacement;
        if (!lang.isEmpty()) {
            replacement = QString("<pre><code class=\"language-%1\">%2</code></pre>").arg(lang).arg(code);
        } else {
            replacement = QString("<pre><code>%1</code></pre>").arg(code);
        }
        
        result.replace(pos, codeBlockRegex.matchedLength(), replacement);
        pos += replacement.length();
    }
    
    return result;
}

QString MarkdownParser::parseLinks(const QString &text)
{
    QString result = text;
    // [text](url)
    result.replace(QRegExp("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    return result;
}

QString MarkdownParser::parseImages(const QString &text)
{
    QString result = text;
    // ![alt](url)
    result.replace(QRegExp("!\\[([^\\]]*)\\]\\(([^)]+)\\)"), "<img src=\"\\2\" alt=\"\\1\"/>");
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
        QRegExp bulletRegex("^\\s*[-*+]\\s+(.*)$");
        
        // 检查有序列表
        QRegExp numberedRegex("^\\s*\\d+\\.\\s+(.*)$");
        
        if (bulletRegex.exactMatch(line)) {
            if (!inBulletList) {
                outputLines << "<ul>";
                inBulletList = true;
                inNumberedList = false;
            }
            outputLines << QString("<li>%1</li>").arg(bulletRegex.cap(1));
        } else if (numberedRegex.exactMatch(line)) {
            if (!inNumberedList) {
                if (inBulletList) {
                    outputLines << "</ul>";
                    inBulletList = false;
                }
                outputLines << "<ol>";
                inNumberedList = true;
            }
            outputLines << QString("<li>%1</li>").arg(numberedRegex.cap(1));
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
    
    return outputLines.join("\n");
}

QString MarkdownParser::parseBlockquotes(const QString &text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList outputLines;
    
    bool inBlockquote = false;
    
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i];
        
        QRegExp quoteRegex("^&gt;\\s*(.*)$");
        
        if (quoteRegex.exactMatch(line)) {
            if (!inBlockquote) {
                outputLines << "<blockquote>";
                inBlockquote = true;
            }
            outputLines << quoteRegex.cap(1);
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
    
    return outputLines.join("\n");
}

QString MarkdownParser::parseHorizontalRules(const QString &text)
{
    QString result = text;
    // ---, ***, ___
    result.replace(QRegExp("^---+$"), "<hr/>");
    result.replace(QRegExp("^\\*\\*\\*+$"), "<hr/>");
    result.replace(QRegExp("^___+$"), "<hr/>");
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
    
    return outputLines.join("\n");
}

QString MarkdownParser::parseTables(const QString &text)
{
    QString result = text;
    QStringList lines = result.split('\n');
    QStringList outputLines;
    
    int i = 0;
    while (i < lines.size()) {
        QString line = lines[i];
        
        // 检查是否是表格分隔行 (|---|---| 或 |---:|:---| 等)
        QRegExp separatorRegex("^\\s*\\|[\\s\\-:\\|]+\\|\\s*$");
        
        if (separatorRegex.exactMatch(line) && i > 0) {
            // 找到表格，开始解析
            QStringList tableRows;
            
            // 第一行是表头
            QString headerLine = lines[i - 1];
            QStringList headers = parseTableRow(headerLine, true);
            tableRows << QString("<tr>%1</tr>").arg(headers.join(""));
            
            // 跳过分隔行
            i++;
            
            // 解析数据行
            while (i < lines.size()) {
                QString dataLine = lines[i];
                QRegExp dataRowRegex("^\\s*\\|.*\\|\\s*$");
                
                if (dataRowRegex.exactMatch(dataLine)) {
                    QStringList cells = parseTableRow(dataLine, false);
                    tableRows << QString("<tr>%1</tr>").arg(cells.join(""));
                    i++;
                } else {
                    break;
                }
            }
            
            // 生成完整的表格 HTML
            QString tableHtml = "<table>\n<thead>\n" + tableRows[0] + "\n</thead>\n<tbody>\n";
            for (int j = 1; j < tableRows.size(); j++) {
                tableHtml += tableRows[j] + "\n";
            }
            tableHtml += "</tbody>\n</table>";
            
            // 替换输出中的表头行和分隔行
            if (!outputLines.isEmpty()) {
                outputLines.removeLast(); // 移除之前添加的表头行
            }
            outputLines << tableHtml;
        } else {
            outputLines << line;
            i++;
        }
    }
    
    return outputLines.join("\n");
}

QStringList MarkdownParser::parseTableRow(const QString &row, bool isHeader)
{
    QStringList cells;
    // Qt 4.8 compatibility: split() without flags keeps empty parts by default
    QStringList parts = row.split('|');
    
    // 过滤首尾空单元格（由行首和行尾的 | 产生）
    int startIdx = 0;
    int endIdx = parts.size();
    
    // 跳过开头的空单元格
    if (parts.size() > 0 && parts[0].trimmed().isEmpty()) {
        startIdx = 1;
    }
    // 跳过结尾的空单元格
    if (parts.size() > 1 && parts.last().trimmed().isEmpty()) {
        endIdx = parts.size() - 1;
    }
    
    for (int i = startIdx; i < endIdx; i++) {
        const QString &part = parts[i];
        QString cellContent = part.trimmed();
        
        // 对单元格内容进行 HTML 转义
        cellContent = htmlEscape(cellContent);
        
        // 解析单元格内的粗体、斜体等格式
        cellContent = parseBold(cellContent);
        cellContent = parseItalic(cellContent);
        cellContent = parseStrikethrough(cellContent);
        cellContent = parseInlineCode(cellContent);
        cellContent = parseLinks(cellContent);
        cellContent = parseImages(cellContent);
        
        if (isHeader) {
            cells << QString("<th>%1</th>").arg(cellContent);
        } else {
            cells << QString("<td>%1</td>").arg(cellContent);
        }
    }
    
    return cells;
}
