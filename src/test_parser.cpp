#include <iostream>
#include <QString>
#include <QRegularExpression>
#include "parser/markdownparser.h"

int main() {
    MarkdownParser parser;
    
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
    
    QString html = parser.parse(markdown);
    std::cout << "=== HTML Output ===" << std::endl;
    std::cout << html.toStdString() << std::endl;
    
    return 0;
}
