// MarkdownParser.h: Markdown 解析器头文件
// 将 Markdown 语法转换为 HTML，不使用第三方库

#pragma once

#include <afx.h>

class CMarkdownParser
{
public:
    CMarkdownParser();
    virtual ~CMarkdownParser();

    // 解析 Markdown 文本为 HTML
    CString Parse(const CString& strMarkdown);

protected:
    // 解析标题 (#, ##, ### 等)
    void ParseHeaders(CString& strHtml);
    
    // 解析粗体和斜体 (**text**, *text*)
    void ParseBoldItalic(CString& strHtml);
    
    // 解析代码块 (`code`)
    void ParseInlineCode(CString& strHtml);
    
    // 解析代码块 (```)
    void ParseCodeBlocks(CString& strHtml);
    
    // 解析链接 [text](url)
    void ParseLinks(CString& strHtml);
    
    // 解析图片 ![alt](url)
    void ParseImages(CString& strHtml);
    
    // 解析列表 (- item, 1. item)
    void ParseLists(CString& strHtml);
    
    // 解析引用 (> quote)
    void ParseBlockquotes(CString& strHtml);
    
    // 解析水平线 (---)
    void ParseHorizontalRules(CString& strHtml);
    
    // 解析段落
    void ParseParagraphs(CString& strHtml);
    
    // HTML 转义
    CString EscapeHtml(const CString& strText);
};
