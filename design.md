# 基于 Qt5.12 的 Markdown 编辑器软件架构方案

## 1. 项目概述

### 1.1 项目目标
开发一款基于 Qt5.12 的跨平台 Markdown 编辑器，采用左右分栏布局，左侧为编辑区，右侧为实时预览区。编辑器需支持行号显示、空白字符显示、自定义字体样式等编辑功能，并支持通过本地 CSS 文件自定义预览样式。

### 1.2 技术栈
- **框架**: Qt 5.12 (C++)
- **Markdown 解析**: QMarkdownParser 或 Custom Parser
- **渲染引擎**: QTextBrowser / QWebEngineView
- **构建系统**: qmake 或 CMake
- **目标平台**: Windows, Linux, macOS

## 2. 系统架构设计

### 2.1 整体架构图
```
┌─────────────────────────────────────────────────────────────┐
│                     MainWindow                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                   Menu & Toolbar                      │  │
│  └───────────────────────────────────────────────────────┘  │
│  ┌─────────────────────┬─────────────────────────────────┐  │
│  │   EditorPane        │      PreviewPane                │  │
│  │  ┌───────────────┐  │  ┌───────────────────────────┐  │  │
│  │  │ QPlainTextEdit│  │  │ QTextBrowser /            │  │  │
│  │  │ + LineNumbers │  │  │ QWebEngineView            │  │  │
│  │  │ + Whitespace  │  │  │                           │  │  │
│  │  │ + Font Config │  │  │ CSS Styling               │  │  │
│  │  └───────────────┘  │  └───────────────────────────┘  │  │
│  └─────────────────────┴─────────────────────────────────┘  │
│  ┌───────────────────────────────────────────────────────┐  │
│  │                  StatusBar                            │  │
│  └───────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
         ▲                        ▲
         │                        │
┌────────┴────────┐    ┌──────────┴──────────┐
│  ConfigManager  │    │   MarkdownParser    │
│  (Settings)     │    │   (Text → HTML)     │
└─────────────────┘    └─────────────────────┘
```

### 2.2 核心模块划分

| 模块名称 | 职责描述 | 关键类 |
|---------|---------|-------|
| **MainWindow** | 主窗口管理，布局控制，菜单工具栏 | `MainWindow` |
| **EditorPane** | 文本编辑区域，行号，空白显示，字体配置 | `CodeEditor`, `LineNumberArea` |
| **PreviewPane** | Markdown 渲染预览，CSS 样式应用 | `PreviewWidget` |
| **MarkdownParser** | Markdown 语法解析，转换为 HTML | `MarkdownParser` |
| **ConfigManager** | 配置文件管理，用户设置持久化 | `ConfigManager` |
| **StyleSheetLoader** | CSS 样式表加载与应用 | `StyleSheetLoader` |

## 3. 详细模块设计

### 3.1 主窗口模块 (MainWindow)

#### 功能职责
- 创建左右分栏布局（QSplitter）
- 管理菜单栏（文件、编辑、视图、帮助）
- 管理工具栏（常用操作快捷按钮）
- 状态栏显示（光标位置，字数统计）

#### 关键接口
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    void initUI();
    void initConnections();
    void loadSettings();
    void saveSettings();
    
private slots:
    void onTextChanged();
    void toggleLineNumbers(bool checked);
    void toggleWhitespace(bool checked);
    void changeFont();
    void loadCSSFile();
    
private:
    CodeEditor *m_editor;
    PreviewWidget *m_preview;
    QSplitter *m_splitter;
    ConfigManager *m_config;
};
```

### 3.2 编辑区模块 (EditorPane)

#### 3.2.1 行号显示 (LineNumberArea)
- 继承自 QWidget
- 重写 `paintEvent()` 绘制行号
- 与 QPlainTextEdit 同步滚动

#### 3.2.2 空白字符显示
- 重写 QPlainTextEdit 的 `paintEvent()`
- 使用 QTextOption::ShowTabsAndSpaces 显示空白
- 可切换显示/隐藏

#### 3.2.3 字体样式配置
- 支持字体家族、大小、粗细配置
- 通过 QFontDialog 选择字体
- 实时应用到编辑器

#### 关键类设计
```cpp
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();
    
    void setLineNumbersVisible(bool visible);
    void setWhitespaceVisible(bool visible);
    void setEditorFont(const QFont &font);
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    
private:
    QWidget *m_lineNumberArea;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor *editor);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    CodeEditor *m_editor;
};
```

### 3.3 预览区模块 (PreviewPane)

#### 功能职责
- 接收 Markdown 文本并解析为 HTML
- 应用 CSS 样式进行渲染
- 支持实时预览和滚动同步

#### 技术方案选择
**方案 A: QTextBrowser (推荐)**
- 轻量级，无需额外依赖
- 支持基本 HTML 渲染
- 适合简单预览需求

**方案 B: QWebEngineView**
- 完整浏览器引擎
- 支持复杂 CSS 和 JavaScript
- 体积较大，启动慢

#### 关键类设计
```cpp
class PreviewWidget : public QTextBrowser {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();
    
    void setMarkdownText(const QString &text);
    void loadCSSFile(const QString &filePath);
    void resetToDefaultCSS();
    
public slots:
    void updatePreview(const QString &markdown);
    
private:
    MarkdownParser *m_parser;
    StyleSheetLoader *m_cssLoader;
    QString m_currentCSS;
    
    void applyStyles();
    QString generateHTML(const QString &htmlContent);
};
```

### 3.4 Markdown 解析器 (MarkdownParser)

#### 功能职责
- 解析 Markdown 语法
- 转换为标准 HTML
- 支持常见语法：标题、列表、链接、图片、代码块等

#### 实现方案
```cpp
class MarkdownParser : public QObject {
    Q_OBJECT
public:
    explicit MarkdownParser(QObject *parent = nullptr);
    
    QString parse(const QString &markdown);
    
private:
    QString parseHeaders(const QString &text);
    QString parseLists(const QString &text);
    QString parseLinks(const QString &text);
    QString parseCodeBlocks(const QString &text);
    QString parseInlineFormatting(const QString &text);
    
    // 或使用第三方库如 QMarkdown
};
```

### 3.5 配置管理模块 (ConfigManager)

#### 功能职责
- 读写配置文件（JSON 或 INI 格式）
- 管理用户偏好设置
- 保存窗口布局、字体、CSS 路径等

#### 配置项设计
```json
{
  "editor": {
    "font_family": "Consolas",
    "font_size": 12,
    "show_line_numbers": true,
    "show_whitespace": false,
    "tab_width": 4
  },
  "preview": {
    "css_file_path": "./styles/default.css",
    "auto_refresh": true,
    "sync_scroll": true
  },
  "window": {
    "splitter_sizes": [800, 800],
    "geometry": {...}
  }
}
```

#### 关键类设计
```cpp
class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager* instance();
    
    void loadConfig(const QString &filePath);
    void saveConfig(const QString &filePath);
    
    // Editor settings
    QFont editorFont() const;
    void setEditorFont(const QFont &font);
    bool showLineNumbers() const;
    void setShowLineNumbers(bool show);
    bool showWhitespace() const;
    void setShowWhitespace(bool show);
    
    // Preview settings
    QString cssFilePath() const;
    void setCssFilePath(const QString &path);
    
private:
    explicit ConfigManager(QObject *parent = nullptr);
    QSettings *m_settings;
};
```

### 3.6 样式表加载器 (StyleSheetLoader)

#### 功能职责
- 读取本地 CSS 文件
- 验证 CSS 语法
- 提供默认样式备用

#### 关键类设计
```cpp
class StyleSheetLoader : public QObject {
    Q_OBJECT
public:
    explicit StyleSheetLoader(QObject *parent = nullptr);
    
    QString loadFromFile(const QString &filePath);
    QString getDefaultCSS();
    bool validateCSS(const QString &css);
    
signals:
    void cssLoaded(const QString &css);
    void cssLoadFailed(const QString &error);
};
```

## 4. 关键技术实现

### 4.1 左右分栏布局
```cpp
m_splitter = new QSplitter(Qt::Horizontal, this);
m_splitter->addWidget(m_editor);
m_splitter->addWidget(m_preview);
m_splitter->setStretchFactor(0, 1);
m_splitter->setStretchFactor(1, 1);
setCentralWidget(m_splitter);
```

### 4.2 行号绘制实现
```cpp
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = round(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + round(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width() - 5, 
                           fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + round(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
```

### 4.3 空白字符显示
```cpp
void CodeEditor::paintEvent(QPaintEvent *event) {
    if (m_showWhitespace) {
        QTextOption option = document()->defaultTextOption();
        option.setTextDirection(Qt::LeftToRight);
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces | 
                       QTextOption::ShowLineAndParagraphSeparators);
        document()->setDefaultTextOption(option);
    }
    QPlainTextEdit::paintEvent(event);
}
```

### 4.4 Markdown 转 HTML 示例
```cpp
QString MarkdownParser::parse(const QString &markdown) {
    QString html = markdown;
    
    // 标题
    html.replace(QRegExp("^###### (.*$)"), "<h6>\\1</h6>");
    html.replace(QRegExp("^##### (.*$)"), "<h5>\\1</h5>");
    html.replace(QRegExp("^#### (.*$)"), "<h4>\\1</h4>");
    html.replace(QRegExp("^### (.*$)"), "<h3>\\1</h3>");
    html.replace(QRegExp("^## (.*$)"), "<h2>\\1</h2>");
    html.replace(QRegExp("^# (.*$)"), "<h1>\\1</h1>");
    
    // 粗体
    html.replace(QRegExp("\\*\\*(.*)\\*\\*"), "<b>\\1</b>");
    
    // 斜体
    html.replace(QRegExp("\\*(.*)\\*"), "<i>\\1</i>");
    
    // 代码块
    html.replace(QRegExp("```([\\s\\S]*?)```"), "<pre><code>\\1</code></pre>");
    
    return html;
}
```

### 4.5 CSS 样式应用
```cpp
void PreviewWidget::loadCSSFile(const QString &filePath) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_currentCSS = file.readAll();
        file.close();
        applyStyles();
    } else {
        QMessageBox::warning(this, tr("Error"), 
                           tr("Failed to load CSS file: %1").arg(filePath));
    }
}

void PreviewWidget::applyStyles() {
    QString html = generateHTML(m_parser->parse(currentMarkdown()));
    QString styledHTML = QString(
        "<!DOCTYPE html><html><head><style>%1</style></head><body>%2</body></html>"
    ).arg(m_currentCSS).arg(html);
    
    setHtml(styledHTML);
}
```

## 5. 文件结构设计

```
markdown-editor/
├── src/
│   ├── main.cpp
│   ├──mainwindow.h/cpp
│   ├── editor/
│   │   ├── codeeditor.h/cpp
│   │   └── linenumberarea.h/cpp
│   ├── preview/
│   │   ├── previewwidget.h/cpp
│   │   └── stylesheetloader.h/cpp
│   ├── parser/
│   │   └── markdownparser.h/cpp
│   └── config/
│       └── configmanager.h/cpp
├── resources/
│   ├── icons/
│   ├── styles/
│   │   ├── default.css
│   │   └── dark.css
│   └── translations/
├── tests/
│   ├── test_markdown_parser.cpp
│   └── test_config_manager.cpp
├── docs/
│   └── user_manual.md
├── CMakeLists.txt (or .pro file)
└── README.md
```

## 6. 依赖管理

### 6.1 Qt 模块依赖
```qmake
QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 可选：如需更强大的网页渲染
# QT += webenginewidgets
```

### 6.2 第三方库（可选）
- **QMarkdown**: 更完善的 Markdown 解析库
- **QtSyntaxHighlighter**: 语法高亮增强

## 7. 扩展性设计

### 7.1 插件架构预留
- 定义插件接口 `IMarkdownPlugin`
- 支持语法高亮插件
- 支持导出格式插件（PDF, HTML 等）

### 7.2 主题系统
- 支持深色/浅色主题切换
- 可配置颜色方案

### 7.3 多语言支持
- 使用 Qt 的国际化机制 (tr())
- 提供翻译文件 (.ts)

## 8. 性能优化策略

### 8.1 渲染优化
- 延迟更新预览（防抖处理，300ms 无输入后更新）
- 增量解析 Markdown（只解析变化部分）
- 异步加载大文件

### 8.2 内存管理
- 合理使用智能指针
- 及时释放大对象
- 避免频繁的字符串拷贝

## 9. 测试策略

### 9.1 单元测试
- Markdown 解析正确性测试
- 配置读写测试
- 边界条件测试

### 9.2 集成测试
- 编辑与预览同步测试
- CSS 加载与应用测试
- 文件操作测试

### 9.3 UI 测试
- 手动测试主要功能流程
- 自动化 UI 测试（可选）

## 10. 开发计划建议

| 阶段 | 时间 | 任务 |
|-----|------|------|
| **Phase 1** | 1 周 | 项目搭建，基础 UI 框架 |
| **Phase 2** | 2 周 | 编辑区功能实现（行号、空白、字体） |
| **Phase 3** | 2 周 | Markdown 解析器与预览区 |
| **Phase 4** | 1 周 | CSS 样式系统与配置管理 |
| **Phase 5** | 1 周 | 测试优化与文档编写 |

## 11. 风险评估

| 风险 | 影响 | 应对措施 |
|-----|------|---------|
| Qt5.12 版本兼容性 | 中 | 严格遵循 Qt5.12 API，避免使用新特性 |
| Markdown 解析复杂度 | 中 | 先实现核心语法，逐步扩展 |
| CSS 渲染兼容性 | 低 | 使用 QTextBrowser 简化渲染 |
| 跨平台差异 | 中 | 各平台充分测试，处理字体和路径差异 |

## 12. 总结

本架构方案采用模块化设计，清晰分离编辑、预览、解析、配置等核心功能，具有良好的可维护性和扩展性。基于 Qt5.12 的成熟技术栈，能够保证跨平台稳定性和性能表现。方案充分考虑了用户需求中的各项功能点，并预留了未来扩展空间。

**建议下一步**: 
1. 确认技术选型（特别是预览组件选择）
2. 细化 Markdown 语法支持范围
3. 确定配置文件格式和存储位置
4. 开始原型开发
