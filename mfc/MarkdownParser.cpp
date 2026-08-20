// MarkdownParser.cpp: Markdown 解析器实现文件
// 将 Markdown 语法转换为 HTML，不使用第三方库

#include "pch.h"
#include "MarkdownParser.h"

CMarkdownParser::CMarkdownParser()
{
}

CMarkdownParser::~CMarkdownParser()
{
}

CString CMarkdownParser::Parse(const CString& strMarkdown)
{
    // 创建完整的 HTML 文档
    CString strHtml = _T("<!DOCTYPE html><html><head><meta charset='utf-8'>");
    strHtml += _T("<style>");
    strHtml += _T("body { font-family: 'Segoe UI', Arial, sans-serif; padding: 20px; line-height: 1.6; }");
    strHtml += _T("h1, h2, h3, h4, h5, h6 { color: #333; margin-top: 1.5em; margin-bottom: 0.5em; }");
    strHtml += _T("h1 { border-bottom: 2px solid #eee; padding-bottom: 0.3em; }");
    strHtml += _T("h2 { border-bottom: 1px solid #eee; padding-bottom: 0.3em; }");
    strHtml += _T("code { background-color: #f4f4f4; padding: 2px 6px; border-radius: 3px; font-family: Consolas, 'Courier New', monospace; }");
    strHtml += _T("pre { background-color: #f4f4f4; padding: 15px; border-radius: 5px; overflow-x: auto; }");
    strHtml += _T("pre code { background-color: transparent; padding: 0; }");
    strHtml += _T("blockquote { border-left: 4px solid #ddd; padding-left: 15px; color: #666; margin-left: 0; }");
    strHtml += _T("ul, ol { padding-left: 30px; }");
    strHtml += _T("li { margin: 0.5em 0; }");
    strHtml += _T("a { color: #0066cc; text-decoration: none; }");
    strHtml += _T("a:hover { text-decoration: underline; }");
    strHtml += _T("img { max-width: 100%; }");
    strHtml += _T("hr { border: none; border-top: 1px solid #ddd; margin: 2em 0; }");
    strHtml += _T("table { border-collapse: collapse; width: 100%; margin: 1em 0; }");
    strHtml += _T("th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }");
    strHtml += _T("th { background-color: #f4f4f4; font-weight: bold; }");
    strHtml += _T("</style></head><body>");
    
    // 首先处理代码块 (需要最先处理以避免内部标记被解析)
    ParseCodeBlocks(strMarkdown);
    
    // HTML 转义 (但保留我们添加的 HTML 标签)
    // 实际上我们在各个解析函数中直接构建 HTML
    
    // 解析各种 Markdown 元素
    ParseHeaders(strMarkdown);
    ParseBoldItalic(strMarkdown);
    ParseInlineCode(strMarkdown);
    ParseLinks(strMarkdown);
    ParseImages(strMarkdown);
    ParseLists(strMarkdown);
    ParseBlockquotes(strMarkdown);
    ParseHorizontalRules(strMarkdown);
    ParseParagraphs(strMarkdown);
    
    strHtml += strMarkdown;
    strHtml += _T("</body></html>");
    
    return strHtml;
}

CString CMarkdownParser::EscapeHtml(const CString& strText)
{
    CString strResult = strText;
    strResult.Replace(_T("&"), _T("&amp;"));
    strResult.Replace(_T("<"), _T("&lt;"));
    strResult.Replace(_T(">"), _T("&gt;"));
    strResult.Replace(_T("\""), _T("&quot;"));
    return strResult;
}

void CMarkdownParser::ParseCodeBlocks(CString& strHtml)
{
    // 处理 ``` 代码块
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("```"), nPos)) != -1)
    {
        int nStart = nPos + 3;
        // 跳过可能的语言标识符
        int nLangEnd = strHtml.Find(_T("\r\n"), nStart);
        if (nLangEnd == -1) nLangEnd = strHtml.Find(_T("\n"), nStart);
        if (nLangEnd == -1) nLangEnd = strHtml.GetLength();
        else nLangEnd++; // 包含换行符
        
        int nEnd = strHtml.Find(_T("```"), nLangEnd);
        if (nEnd == -1) break;
        
        CString strCode = strHtml.Mid(nLangEnd, nEnd - nLangEnd);
        strCode = EscapeHtml(strCode);
        
        CString strReplacement = _T("<pre><code>") + strCode + _T("</code></pre>");
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nEnd + 3);
        nPos += strReplacement.GetLength();
    }
}

void CMarkdownParser::ParseHeaders(CString& strHtml)
{
    // 处理 # 到 ###### 标题
    for (int level = 6; level >= 1; level--)
    {
        CString strPattern;
        for (int i = 0; i < level; i++) strPattern += _T("#");
        strPattern += _T(" ");
        
        int nPos = 0;
        while ((nPos = strHtml.Find(strPattern, nPos)) != -1)
        {
            // 确保在行首
            BOOL bAtLineStart = (nPos == 0);
            if (!bAtStart && nPos > 0)
            {
                TCHAR chPrev = strHtml[nPos - 1];
                bAtLineStart = (chPrev == _T('\r') || chPrev == _T('\n'));
            }
            
            if (bAtLineStart || nPos == 0)
            {
                int nLineEnd = strHtml.Find(_T("\r\n"), nPos);
                if (nLineEnd == -1) nLineEnd = strHtml.Find(_T("\n"), nPos);
                if (nLineEnd == -1) nLineEnd = strHtml.GetLength();
                
                CString strHeader = strHtml.Mid(nPos + level + 1, nLineEnd - nPos - level - 1);
                CString strReplacement;
                strReplacement.Format(_T("<h%d>%s</h%d>"), level, strHeader, level);
                
                // 替换整行
                int nLineStart = nPos;
                if (nPos > 0 && strHtml[nPos-1] == _T('\n')) nLineStart = nPos;
                else if (nPos > 1 && strHtml[nPos-1] == _T('\r') && strHtml[nPos-2] == _T('\n')) nLineStart = nPos;
                
                strHtml = strHtml.Left(nLineStart) + strReplacement + strHtml.Mid(nLineEnd);
                nPos = nLineStart + strReplacement.GetLength();
            }
            else
            {
                nPos++;
            }
        }
    }
}

void CMarkdownParser::ParseBoldItalic(CString& strHtml)
{
    // 处理粗体 **text**
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("**"), nPos)) != -1)
    {
        int nEnd = strHtml.Find(_T("**"), nPos + 2);
        if (nEnd == -1) break;
        
        CString strText = strHtml.Mid(nPos + 2, nEnd - nPos - 2);
        CString strReplacement = _T("<strong>") + strText + _T("</strong>");
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nEnd + 2);
        nPos += strReplacement.GetLength();
    }
    
    // 处理斜体 *text* (但要排除已经在 < 标签内的)
    nPos = 0;
    while ((nPos = strHtml.Find(_T("*"), nPos)) != -1)
    {
        // 检查是否是 ** 的一部分
        if (nPos + 1 < strHtml.GetLength() && strHtml[nPos + 1] == _T('*'))
        {
            nPos += 2;
            continue;
        }
        
        // 查找匹配的结束 *
        int nEnd = nPos + 1;
        while (nEnd < strHtml.GetLength())
        {
            if (strHtml[nEnd] == _T('*') && (nEnd == 0 || strHtml[nEnd-1] != _T('*')))
            {
                // 检查不是 ** 的开始
                if (nEnd + 1 >= strHtml.GetLength() || strHtml[nEnd + 1] != _T('*'))
                {
                    break;
                }
            }
            nEnd++;
        }
        
        if (nEnd >= strHtml.GetLength()) break;
        
        CString strText = strHtml.Mid(nPos + 1, nEnd - nPos - 1);
        CString strReplacement = _T("<em>") + strText + _T("</em>");
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nEnd + 1);
        nPos += strReplacement.GetLength();
    }
}

void CMarkdownParser::ParseInlineCode(CString& strHtml)
{
    // 处理 `code`
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("`"), nPos)) != -1)
    {
        // 检查是否是 ``` 代码块的开始
        if (nPos + 2 < strHtml.GetLength() && 
            strHtml[nPos + 1] == _T('`') && strHtml[nPos + 2] == _T('`'))
        {
            nPos += 3;
            continue;
        }
        
        int nEnd = strHtml.Find(_T("`"), nPos + 1);
        if (nEnd == -1) break;
        
        CString strCode = strHtml.Mid(nPos + 1, nEnd - nPos - 1);
        strCode = EscapeHtml(strCode);
        CString strReplacement = _T("<code>") + strCode + _T("</code>");
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nEnd + 1);
        nPos += strReplacement.GetLength();
    }
}

void CMarkdownParser::ParseLinks(CString& strHtml)
{
    // 处理 [text](url)
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("["), nPos)) != -1)
    {
        int nTextEnd = strHtml.Find(_T("]"), nPos);
        if (nTextEnd == -1) break;
        
        if (nTextEnd + 1 >= strHtml.GetLength() || strHtml[nTextEnd + 1] != _T('('))
        {
            nPos++;
            continue;
        }
        
        int nUrlEnd = strHtml.Find(_T(")"), nTextEnd + 2);
        if (nUrlEnd == -1) break;
        
        CString strText = strHtml.Mid(nPos + 1, nTextEnd - nPos - 1);
        CString strUrl = strHtml.Mid(nTextEnd + 2, nUrlEnd - nTextEnd - 2);
        CString strReplacement = _T("<a href='") + strUrl + _T("'>") + strText + _T("</a>");
        
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nUrlEnd + 1);
        nPos += strReplacement.GetLength();
    }
}

void CMarkdownParser::ParseImages(CString& strHtml)
{
    // 处理 ![alt](url)
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("![", nPos))) != -1)
    {
        int nAltEnd = strHtml.Find(_T("]"), nPos + 2);
        if (nAltEnd == -1) break;
        
        if (nAltEnd + 1 >= strHtml.GetLength() || strHtml[nAltEnd + 1] != _T('('))
        {
            nPos++;
            continue;
        }
        
        int nUrlEnd = strHtml.Find(_T(")"), nAltEnd + 2);
        if (nUrlEnd == -1) break;
        
        CString strAlt = strHtml.Mid(nPos + 2, nAltEnd - nPos - 2);
        CString strUrl = strHtml.Mid(nAltEnd + 2, nUrlEnd - nAltEnd - 2);
        CString strReplacement = _T("<img src='") + strUrl + _T("' alt='") + strAlt + _T("' />");
        
        strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nUrlEnd + 1);
        nPos += strReplacement.GetLength();
    }
}

void CMarkdownParser::ParseLists(CString& strHtml)
{
    // 处理无序列表 (- item)
    CStringArray arrLines;
    int nStart = 0;
    while (nStart < strHtml.GetLength())
    {
        int nLineEnd = strHtml.Find(_T("\r\n"), nStart);
        if (nLineEnd == -1) nLineEnd = strHtml.GetLength();
        
        CString strLine = strHtml.Mid(nStart, nLineEnd - nStart);
        arrLines.Add(strLine);
        
        if (nLineEnd >= strHtml.GetLength()) break;
        nStart = nLineEnd + 2;
    }
    
    CString strResult;
    BOOL bInList = FALSE;
    
    for (int i = 0; i < arrLines.GetCount(); i++)
    {
        CString strLine = arrLines[i];
        strLine.TrimLeft();
        
        if (strLine.Left(2) == _T("- ") || strLine.Left(2) == _T("* "))
        {
            if (!bInList)
            {
                strResult += _T("<ul>");
                bInList = TRUE;
            }
            strResult += _T("<li>") + strLine.Mid(2) + _T("</li>");
        }
        else if (!strLine.IsEmpty() && strLine[0] >= _T('0') && strLine[0] <= _T('9') && strLine.Find(_T(". ")) != -1)
        {
            if (!bInList)
            {
                strResult += _T("<ol>");
                bInList = TRUE;
            }
            int nDotPos = strLine.Find(_T(". "));
            strResult += _T("<li>") + strLine.Mid(nDotPos + 2) + _T("</li>");
        }
        else
        {
            if (bInList)
            {
                // 检查前一行是否以 </li> 结尾
                if (strResult.Right(5) == _T("<li>"))
                    strResult = strResult.Left(strResult.GetLength() - 5) + _T("</li>");
                
                strResult += strLine.IsEmpty() ? _T("") : (strLine + _T("\r\n"));
                if (strResult.Find(_T("<ul>")) != -1 && strResult.Find(_T("</ul>")) == -1)
                    strResult += _T("</ul>");
                if (strResult.Find(_T("<ol>")) != -1 && strResult.Find(_T("</ol>")) == -1)
                    strResult = strResult + _T("</ol>");
                bInList = FALSE;
            }
            else
            {
                strResult += strLine.IsEmpty() ? _T("") : (strLine + _T("\r\n"));
            }
        }
    }
    
    if (bInList)
    {
        if (strResult.Find(_T("<ul>")) != -1 && strResult.Find(_T("</ul>")) == -1)
            strResult += _T("</ul>");
        if (strResult.Find(_T("<ol>")) != -1 && strResult.Find(_T("</ol>")) == -1)
            strResult += _T("</ol>");
    }
    
    strHtml = strResult;
}

void CMarkdownParser::ParseBlockquotes(CString& strHtml)
{
    // 处理 > 引用
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("> "), nPos)) != -1)
    {
        // 检查是否在行首
        BOOL bAtLineStart = (nPos == 0);
        if (!bAtLineStart && nPos > 0)
        {
            TCHAR chPrev = strHtml[nPos - 1];
            bAtLineStart = (chPrev == _T('\r') || chPrev == _T('\n'));
        }
        
        if (bAtLineStart || nPos == 0)
        {
            int nLineEnd = strHtml.Find(_T("\r\n"), nPos);
            if (nLineEnd == -1) nLineEnd = strHtml.GetLength();
            
            CString strQuote = strHtml.Mid(nPos + 2, nLineEnd - nPos - 2);
            CString strReplacement = _T("<blockquote>") + strQuote + _T("</blockquote>");
            
            strHtml = strHtml.Left(nPos) + strReplacement + strHtml.Mid(nLineEnd);
            nPos += strReplacement.GetLength();
        }
        else
        {
            nPos++;
        }
    }
}

void CMarkdownParser::ParseHorizontalRules(CString& strHtml)
{
    // 处理 --- 水平线
    int nPos = 0;
    while ((nPos = strHtml.Find(_T("---"), nPos)) != -1)
    {
        // 检查是否在行首
        BOOL bAtLineStart = (nPos == 0);
        if (!bAtLineStart && nPos > 0)
        {
            TCHAR chPrev = strHtml[nPos - 1];
            bAtLineStart = (chPrev == _T('\r') || chPrev == _T('\n'));
        }
        
        if (bAtLineStart || nPos == 0)
        {
            int nLineEnd = strHtml.Find(_T("\r\n"), nPos);
            if (nLineEnd == -1) nLineEnd = strHtml.GetLength();
            
            // 检查是否全是 -
            CString strLine = strHtml.Mid(nPos, nLineEnd - nPos);
            BOOL bAllDashes = TRUE;
            for (int i = 0; i < strLine.GetLength(); i++)
            {
                if (strLine[i] != _T('-') && strLine[i] != _T(' ') && strLine[i] != _T('\r') && strLine[i] != _T('\n'))
                {
                    bAllDashes = FALSE;
                    break;
                }
            }
            
            if (bAllDashes && strLine.GetLength() >= 3)
            {
                strHtml = strHtml.Left(nPos) + _T("<hr/>") + strHtml.Mid(nLineEnd);
                nPos += 5;
            }
            else
            {
                nPos++;
            }
        }
        else
        {
            nPos++;
        }
    }
}

void CMarkdownParser::ParseParagraphs(CString& strHtml)
{
    // 简单的段落处理：将连续的非空行包裹在 <p> 标签中
    // 这个实现比较简化，实际可能需要更复杂的逻辑
    
    // 将双换行符替换为段落结束
    strHtml.Replace(_T("\r\n\r\n"), _T("</p><p>"));
    strHtml.Replace(_T("\n\n"), _T("</p><p>"));
    
    // 在开头和结尾添加 p 标签（如果没有其他块级标签）
    if (!strHtml.IsEmpty() && strHtml.Left(1) != _T("<"))
    {
        strHtml = _T("<p>") + strHtml;
    }
    if (!strHtml.IsEmpty() && strHtml.Right(4) != _T("</p>") && 
        strHtml.Right(5) != _T("</h1>") && strHtml.Right(5) != _T("</h2>") &&
        strHtml.Right(5) != _T("</h3>") && strHtml.Right(7) != _T("</blockquote>"))
    {
        strHtml += _T("</p>");
    }
}
