import sys
sys.path.insert(0, '.')

# 测试 Markdown 解析器的代码块解析功能
test_markdown = """
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
"""

print("Test markdown input:")
print(test_markdown)
print("\n" + "="*50 + "\n")
