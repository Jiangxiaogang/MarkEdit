# MarkEdit — Markdown 编辑器软件架构方案

## 1. 项目概述

### 1.1 项目目标
基于 Qt 5.4 开发一款跨平台 Markdown 编辑器，采用左右分栏布局（`QSplitter`）：左侧为源码编辑区，右侧为实时预览区。支持行号显示、空白字符显示、语法高亮、自定义字体与 Tab 宽度等编辑功能，并支持通过本地 CSS 文件自定义预览样式。

### 1.2 技术栈
- **框架**: Qt 5.4 (C++11)
- **Markdown 解析**: 自研行式解析器 `MarkdownParser`（基于 `QRegularExpression`，无需第三方库）
- **渲染引擎**: QtWebKit（`QWebView`）
- **构建系统**: qmake（`MarkEdit.pro`）
- **配置持久化**: `QSettings`（INI 格式）
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
│  │     CodeEditor      │        PreviewWidget            │  │
│  │  ┌───────────────┐  │  ┌───────────────────────────┐  │  │
│  │  │ QPlainTextEdit│  │  │ QWebView (QtWebKit)       │  │  │
│  │  │ + LineNumbers │  │  │ + CSS Styling             │  │  │
│  │  │ + Whitespace  │  │  │ + Scroll Sync             │  │  │
│  │  │ + Highlighter │  │  │                           │  │  │
│  │  │ + Font/TabW   │  │  │                           │  │  │
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
│  (QSettings)    │    │   (Text → HTML)     │
└─────────────────┘    └─────────────────────┘
```

### 2.2 核心模块划分

| 模块名称 | 职责描述 | 关键类 |
|---------|---------|-------|
| **MainWindow** | 主窗口管理，QSplitter 布局，菜单/工具栏/状态栏 | `MainWindow` |
| **EditorPane** | 文本编辑，行号，空白显示，语法高亮，字体/Tab 宽度 | `CodeEditor`, `LineNumberArea`, `MarkdownHighlighter` |
| **PreviewPane** | Markdown 渲染预览，CSS 样式，滚动同步 | `PreviewWidget` |
| **MarkdownParser** | Markdown 语法解析，转换为 HTML | `MarkdownParser` |
| **ConfigManager** | 用户设置持久化（QSettings 单例） | `ConfigManager` |
| **StyleSheetLoader** | CSS 样式表加载、验证与默认样式 | `StyleSheetLoader` |
| **Dialogs** | 查找替换、偏好设置、插入链接/图片、关于 | `FindReplaceDialog`, `SettingsDialog`, `InsertDialog`, `AboutDialog` |

## 3. 详细模块设计

### 3.1 主窗口模块 (MainWindow)

#### 功能职责
- 创建左右分栏布局（`QSplitter`，默认 1:1 分隔）
- 管理菜单栏（文件、编辑、视图、格式、工具、帮助）
- 管理工具栏与状态栏（光标位置、字数统计）
- 双向滚动同步（编辑区 ↔ 预览区）

#### 关键接口
```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUI();
    void initMenus();
    void initToolbar();
    void initStatusBar();
    void initConnections();
    void loadSettings();
    void saveSettings();
    void applyConfigToUi();

    // 文件 / 编辑 / 视图 / 格式 / 工具 / 帮助 操作
    // 滚动同步: syncScrollFromEditor(int), syncScrollFromPreview(int)
    // 上下文菜单: editorContextMenu(pos), previewContextMenu(pos)

    CodeEditor *m_editor;
    PreviewWidget *m_preview;
    QSplitter *m_splitter;
    ConfigManager *m_config;
    StyleSheetLoader *m_styleLoader;
    QTimer *m_previewTimer;
    QString m_currentFile;
    bool m_syncing;
    bool m_splitSet;
};
```

### 3.2 编辑区模块 (CodeEditor)

#### 功能职责
- 继承 `QPlainTextEdit`，提供 Markdown 源码编辑
- 行号 gutter（`LineNumberArea`）
- 当前行高亮
- 空白字符显示（`QTextOption::ShowTabsAndSpaces`）
- 语法高亮（`MarkdownHighlighter`）
- 自定义字体与 Tab 宽度

#### 关键接口
```cpp
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    void setLineNumbersVisible(bool visible);
    void setWhitespaceVisible(bool visible);
    void setSyntaxHighlightingEnabled(bool enabled);
    void setTabWidth(int width);
    void setEditorFont(const QFont &font);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    LineNumberArea *m_lineNumberArea;
    MarkdownHighlighter *m_highlighter;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    bool m_syntaxHighlighting;
};
```

#### 3.2.1 行号显示 (LineNumberArea)
- 继承 `QWidget`，作为编辑器的左 margin 子控件
- `sizeHint()` 返回 `CodeEditor::lineNumberAreaWidth()`
- `paintEvent()` 委托给 `CodeEditor::lineNumberAreaPaintEvent()`
- 与 `QPlainTextEdit` 通过 `blockCountChanged` / `updateRequest` 同步滚动

#### 3.2.2 语法高亮 (MarkdownHighlighter)
- 继承 `QSyntaxHighlighter`，仅改变文字前景色，不改字体/字号/样式
- 支持标题、代码块（含跨行围栏代码块状态机）、引用、列表、水平线、粗体、斜体、删除线、链接、表格
- 通过 `setDocument()` 挂载/卸载实现启停（禁用时清除已着色格式）

#### 3.2.3 Tab 宽度
- `setTabWidth(int width)` 将"空格数"换算为像素，调用 `setTabStopWidth()`
- 依赖当前字体宽度，需在字体设置之后调用

### 3.3 预览区模块 (PreviewWidget)

#### 功能职责
- 继承 `QWebView`（QtWebKit），渲染解析后的 HTML
- 应用 CSS 样式（支持自定义文件 + 内置默认/暗色样式）
- 双向比例滚动同步
- 链接点击委托给系统浏览器（`QDesktopServices`）

#### 关键接口
```cpp
class PreviewWidget : public QWebView {
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();

    void setCSS(const QString &css);
    void setMarkdown(const QString &markdown);
    void refresh();
    int scrollMaximum() const;
    int scrollValue() const;
    void setScrollRatio(float ratio);

signals:
    void scrolled(int value);
    void cssFailed(const QString &error);

private:
    QString generateHtml(const QString &body) const;
    void applyRatio(float ratio);
    MarkdownParser *m_parser;
    StyleSheetLoader *m_loader;
    QTimer *m_scrollTimer;
    int m_lastScroll;
    float m_pendingRatio;
    bool m_emitScroll;
};
```

#### 3.3.1 滚动同步机制
- **编辑器 → 预览**: 编辑器 `verticalScrollBar()->valueChanged` → `MainWindow::syncScrollFromEditor()`，按比例调用 `setScrollRatio()`
- **预览 → 编辑器**: `PreviewWidget` 内部 100ms 定时器轮询 `QWebFrame::scrollBarValue()` 检测变化，`emit scrolled(value)` → `MainWindow::syncScrollFromPreview()`，按比例设置编辑器滚动条
- `m_syncing` 标志防止双向递归
- `setHtml()` 异步加载，`loadFinished` 时重新应用 `m_pendingRatio` 以保持滚动位置

### 3.4 Markdown 解析器 (MarkdownParser)

#### 功能职责
- 将 Markdown 文本解析为 HTML 片段（body 内容）
- 行式（line-based）解析，鲁棒性优先，非 100% CommonMark 兼容

#### 支持的语法
| 语法 | 说明 |
|------|------|
| 标题 | ATX 风格 `#` ~ `######` |
| 粗体 / 斜体 / 删除线 | `**text**`, `__text__`, `*text*`, `_text_`, `~~text~~` |
| 行内代码 / 围栏代码块 | `` `code` `` / ```` ``` ```` 或 `~~~`（支持语言标注） |
| 无序 / 有序列表 | `-`, `*`, `+` / `1.` |
| 引用块 | `> text` |
| 链接 / 图片 | `[text](url)` / `![alt](url)` |
| 水平线 | `---`, `***`, `___` |
| 段落 | 连续非空行 |
| 表格 | GFM 管道表格（支持列对齐 `:---`, `---:`） |

#### 关键接口
```cpp
class MarkdownParser : public QObject {
    Q_OBJECT
public:
    explicit MarkdownParser(QObject *parent = nullptr);
    QString parse(const QString &markdown) const;

private:
    QString escapeHtml(const QString &text) const;
    QString parseInline(const QString &text) const;
    QString parseBlock(const QStringList &lines, int &i) const;
    // parseCodeBlock / parseList / parseBlockQuote / parseHeading /
    // parseHorizontalRule / parseParagraph / parseTable ...
};
```

### 3.5 配置管理模块 (ConfigManager)

#### 功能职责
- 单例模式，全局共享配置实例
- 基于 `QSettings`（INI 格式）读写
- 管理编辑器、预览、窗口状态与最近文件列表
- 配置变更时 `emit configurationChanged()`

#### 配置项（QSettings 键值）
```
[editor]
  font                 = Consolas, 12pt
  show_line_numbers    = true
  show_whitespace      = false
  syntax_highlighting  = true
  tab_width            = 4

[preview]
  css_file_path        = :/styles/default.css
  sync_scroll          = true
  auto_refresh         = true

[window]
  geometry             = <QByteArray>
  state                = <QByteArray>

[recent]
  files                = <QStringList>
```

#### 关键接口
```cpp
class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager *instance();
    void loadConfig();
    void saveConfig();

    // 编辑器设置
    QFont editorFont() const; void setEditorFont(const QFont &font);
    bool showLineNumbers() const; void setShowLineNumbers(bool show);
    bool showWhitespace() const; void setShowWhitespace(bool show);
    bool showSyntaxHighlighting() const; void setShowSyntaxHighlighting(bool show);
    int tabWidth() const; void setTabWidth(int width);

    // 预览设置
    QString cssFilePath() const; void setCssFilePath(const QString &path);
    bool syncScroll() const; void setSyncScroll(bool sync);
    bool autoRefresh() const; void setAutoRefresh(bool refresh);

    // 窗口状态 / 最近文件 ...
signals:
    void configurationChanged();
};
```

### 3.6 样式表加载器 (StyleSheetLoader)

#### 功能职责
- 读取本地 CSS 文件（UTF-8）
- 校验 CSS（花括号配平）
- 提供内置默认（浅色）与暗色样式，加载失败时回退到默认样式

#### 关键接口
```cpp
class StyleSheetLoader : public QObject {
    Q_OBJECT
public:
    explicit StyleSheetLoader(QObject *parent = nullptr);
    QString loadFromFile(const QString &filePath);
    static QString getDefaultCSS();
    static QString getDarkCSS();
    bool validateCSS(const QString &css);

signals:
    void cssLoaded(const QString &css);
    void cssLoadFailed(const QString &error);
};
```

### 3.7 对话框模块 (Dialogs)

| 类 | 职责 |
|----|------|
| `FindReplaceDialog` | 模态查找/替换，支持大小写敏感、整词匹配、全部替换 |
| `SettingsDialog` | 偏好设置（编辑器 Tab + 预览 Tab），绑定 `ConfigManager` |
| `InsertDialog` | 插入链接 / 图片的输入提示框（`kind` 区分模式） |
| `AboutDialog` | "关于"对话框，显示版本、Qt 版本与许可证信息 |

#### SettingsDialog 布局
- **Editor Tab**: 字体、字号、Tab 宽度、显示行号、显示空白字符、语法高亮
- **Preview Tab**: CSS 文件路径、同步滚动、自动刷新预览

## 4. 关键技术实现

### 4.1 左右分栏布局（默认 1:1）
```cpp
m_splitter = new QSplitter(Qt::Horizontal, this);
m_splitter->addWidget(m_editor);
m_splitter->addWidget(m_preview);
m_splitter->setStretchFactor(0, 1);
m_splitter->setStretchFactor(1, 1);
setCentralWidget(m_splitter);

// showEvent 首次显示时强制 1:1
void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    if (!m_splitSet) {
        m_splitSet = true;
        int half = m_splitter->width() / 2;
        m_splitter->setSizes(QList<int>() << half << half);
    }
}
```

### 4.2 行号绘制
```cpp
void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    QTextBlock block = firstVisibleBlock();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    // 遍历可见块绘制行号 ...
}
```

### 4.3 空白字符显示
```cpp
void CodeEditor::setWhitespaceVisible(bool visible) {
    QTextOption option = document()->defaultTextOption();
    if (visible)
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces |
                        QTextOption::ShowLineAndParagraphSeparators);
    else
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces &
                        ~QTextOption::ShowLineAndParagraphSeparators);
    document()->setDefaultTextOption(option);
}
```

### 4.4 预览渲染与滚动同步
```cpp
// PreviewWidget::refresh() 生成完整 HTML 并异步加载
QString html = QString(
    "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
    "<style>%1</style></head><body>%2</body></html>")
    .arg(m_css).arg(body);
setHtml(html);

// 预览滚动检测（100ms 定时器轮询）
void PreviewWidget::onScrollTimeout() {
    if (!m_emitScroll) return;
    int v = scrollValue();
    if (v != m_lastScroll) {
        m_lastScroll = v;
        emit scrolled(v);
    }
}

// 编辑器侧同步（MainWindow）
void MainWindow::syncScrollFromEditor(int value) {
    if (m_syncing || !m_config->syncScroll() || !m_preview->isVisible()) return;
    int max = m_editor->verticalScrollBar()->maximum();
    float ratio = max > 0 ? (float)value / max : 0.0f;
    m_syncing = true;
    m_preview->setScrollRatio(ratio);
    m_syncing = false;
}
```

### 4.5 语法高亮应用
```cpp
// MarkdownHighlighter::highlightBlock() 按块着色，仅设置前景色
void MarkdownHighlighter::highlightBlock(const QString &text) {
    int state = previousBlockState();
    // 围栏代码块状态机、标题、引用、列表、粗体/斜体/删除线、链接、表格 ...
    setFormat(start, length, m_codeFmt);
    setCurrentBlockState(...);
}
```

## 5. 文件结构

```
MarkEdit/
├── src/
│   ├── main.cpp
│   ├── mainwindow.h / mainwindow.cpp
│   ├── codeeditor.h / codeeditor.cpp
│   ├── linenumberarea.h / linenumberarea.cpp
│   ├── markdownhighlighter.h / markdownhighlighter.cpp
│   ├── previewwidget.h / previewwidget.cpp
│   ├── markdownparser.h / markdownparser.cpp
│   ├── configmanager.h / configmanager.cpp
│   ├── stylesheetloader.h / stylesheetloader.cpp
│   ├── findreplacedialog.h / findreplacedialog.cpp
│   ├── settingsdialog.h / settingsdialog.cpp
│   ├── insertdialog.h / insertdialog.cpp
│   └── aboutdialog.h / aboutdialog.cpp
├── resources/
│   ├── resources.qrc
│   └── styles/
│       ├── default.css
│       └── dark.css
├── tests/
│   ├── test_markdown_parser.cpp
│   └── test_config_manager.cpp
├── docs/
│   └── user_manual.md
├── MarkEdit.pro
├── build.bat
└── readme.md
```

## 6. 扩展性设计

### 6.1 主题系统
- `StyleSheetLoader` 提供浅色（`getDefaultCSS`）与暗色（`getDarkCSS`）两套内置样式
- 支持通过本地 CSS 文件自定义预览样式

### 6.2 多语言支持
- 使用 Qt 国际化机制（`tr()`），可扩展 `.ts` 翻译文件

## 7. 性能优化策略

- **预览延迟更新**: 编辑器文本变更后通过 300ms 单次定时器（`QTimer`）防抖，避免每次击键都重新渲染
- **语法高亮**: `QSyntaxHighlighter` 按块增量高亮，仅处理变更块
- **滚动同步**: 预览侧采用定时器轮询（100ms），避免高频信号开销

## 8. 测试策略

- **单元测试**: `tests/test_markdown_parser.cpp`（解析正确性）、`tests/test_config_manager.cpp`（配置读写与最近文件去重）
- **集成测试**: 编辑与预览同步、CSS 加载与应用、文件读写
- **UI 测试**: 手动测试主要功能流程
