# Markdown 语法测试文档

本文档用于测试 Markdown 渲染器对各类语法的支持情况。

---

## 1. 标题语法 (Headings)

# H1 一级标题
## H2 二级标题
### H3 三级标题
#### H4 四级标题
##### H5 五级标题
###### H6 六级标题

---

## 2. 段落与换行 (Paragraphs)

这是第一段文字。Markdown 中，连续的文本行会被视为同一个段落。

这是第二段文字。  
注意这行末尾有两个空格，所以这是一个 **软换行**（换行但不分段）。

---

## 3. 强调与修饰 (Emphasis)

- *斜体* 或 _斜体_
- **粗体** 或 __粗体__
- ***粗斜体*** 或 ___粗斜体___
- ~~删除线~~
- <u>下划线（使用 HTML 标签）</u>
- 这是 ^上标^ (某些方言支持，如 Typora)
- 这是 ~下标~ (某些方言支持)

---

## 4. 列表 (Lists)

### 无序列表 (Unordered)
- 项目 1
- 项目 2
  - 子项目 2.1
  - 子项目 2.2
- 项目 3

### 有序列表 (Ordered)
1. 第一步
2. 第二步
   1. 子步骤 2.1
   2. 子步骤 2.2
3. 第三步

### 混合列表 (Mixed)
- 无序项目
  1. 有序子项目
  2. 有序子项目
- 另一个无序项目

---

## 5. 任务列表 (Task Lists)

- [x] 已完成的任务
- [ ] 未完成的任务
- [ ] 待办事项 1
  - [x] 子任务 1.1
  - [ ] 子任务 1.2

---

## 6. 代码 (Code)

### 行内代码 (Inline)
使用 `console.log('Hello World')` 来输出日志。

### 代码块 (Fenced Code Block)
```javascript
// 这是一个 JavaScript 代码块
function greet(name) {
  return `Hello, ${name}!`;
}
console.log(greet('Markdown'));
```

```python
# 这是一个 Python 代码块
def hello():
    print("Hello, World!")
```

---

## 7. 引用 (Blockquotes)

> 这是一级引用。
> > 这是嵌套的二级引用。
> > > 这是三级引用。
>
> 回到一级引用。

---

## 8. 链接 (Links)

- [这是一个行内链接](https://www.example.com)
- [这是一个带有标题的链接](https://www.example.com "鼠标悬停时显示此标题")
- [这是一个相对路径链接](./other-file.md)
- 这是一个自动链接：<https://www.example.com>

---

## 9. 图片 (Images)

![这是图片的替代文本](https://via.placeholder.com/150 "可选图片标题")

![本地图片示例](./images/logo.png)

---

## 10. 表格 (Tables)

| 左对齐 | 居中对齐 | 右对齐 |
| :----- | :------: | -----: |
| 苹果   |  红色    |   5.00 |
| 香蕉   |  黄色    |   3.50 |
| 葡萄   |  紫色    |   6.99 |

---

## 11. 分隔线 (Horizontal Rules)

以下是三种分隔线语法，效果相同：

---

***

___

---

## 12. 脚注 (Footnotes)

这里是一个脚注示例[^1]，这是第二个脚注[^note]。

[^1]: 这是脚注的内容。
[^note]: 这是另一个脚注的内容。

---

## 13. 转义字符 (Escaping)

使用反斜杠来转义特殊字符：\* \_ \# \[ \] \( \) \> \| \. \! \-

---

## 14. 嵌入 HTML / 内联样式

<span style="color: red;">这是红色的文字（使用 HTML 标签）。</span>

<div style="background: #f0f0f0; padding: 10px; border-radius: 5px;">
  这是一个使用 div 和 style 属性的区块。
</div>

---

## 15. 表情符号 (Emoji) (GFM 支持)

:smile: :heart: :rocket: :+1: :-1:

---

## 16. 高亮与下划线（部分编辑器支持）

==高亮文本== (需要开启 Highlight 扩展)

---

## 17. 定义列表 (Definition Lists) (部分方言支持)

术语 1
: 这是术语 1 的定义。

术语 2
: 这是术语 2 的第一个定义。
: 这是术语 2 的第二个定义。

---

## 18. 目录 (TOC) (部分编辑器自动生成)

<!-- TOC -->
*本文档不自动生成目录，仅作语法占位*
<!-- /TOC -->

---

## 19. 数学公式 (LaTeX) (需要 MathJax 支持)

行内公式：\( E = mc^2 \)

块级公式：

$$
\int_{-\infty}^{\infty} e^{-x^2} dx = \sqrt{\pi}
$$

---

**测试结束。** 如果以上所有内容都能正确渲染，则说明你的 Markdown 编辑器/渲染器功能非常全面。
