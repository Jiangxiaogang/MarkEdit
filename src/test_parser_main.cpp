#include <QDebug>
#include <QString>
#include <QRegularExpression>

// 直接复制解析逻辑进行测试
QString parseCodeBlocks(const QString &text)
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

int main()
{
    QString markdown = R"(
# 测试代码块

这是普通文本。

```cpp
#include <iostream>
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```

这是另一个代码块：

```python
def hello():
    print("Hello from Python")
```

行内代码：`print("test")`
)";
    
    qDebug() << "=== Input Markdown ===";
    qDebug() << markdown;
    qDebug() << "\n=== HTML Output ===";
    
    QString html = parseCodeBlocks(markdown);
    qDebug() << html;
    
    // 检查代码块是否正确解析
    qDebug() << "\n=== Test Results ===";
    if (html.contains("<pre><code class=\"language-cpp\">")) {
        qDebug() << "✓ C++ code block with language class: FOUND";
    } else {
        qDebug() << "✗ C++ code block with language class: NOT FOUND";
    }
    
    if (html.contains("<pre><code class=\"language-python\">")) {
        qDebug() << "✓ Python code block with language class: FOUND";
    } else {
        qDebug() << "✗ Python code block with language class: NOT FOUND";
    }
    
    if (html.contains("&lt;iostream&gt;")) {
        qDebug() << "✓ HTML escaping in code block: WORKING";
    } else {
        qDebug() << "✗ HTML escaping in code block: NOT WORKING";
    }
    
    return 0;
}
