# MarkEdit

基于 Qt 的跨平台 Markdown 编辑器，采用左右分栏布局：左侧为 Markdown 源码编辑区，右侧为实时渲染预览区。编辑器支持行号、空白字符显示、可配置配色的语法高亮、字体与 Tab 宽度设置、编码检测与转换；预览区基于 QtWebKit 渲染，样式可由本地 CSS 自定义，并支持编辑区与预览区双向滚动同步。

**123**aaaaaaa**123**
__123__aaaaaaa__123__ 


---

## 1. 软件环境

### 1.1 运行 / 构建平台
- **操作系统**：理论上可以跨平台编译，但目标主要针对Windows 构建。
- **编译器**：MinGW（C++11 / C11）。CMake 生成的 cmark-gfm 静态库以 `.a` 形式随工程提供。

### 1.2 开发框架与依赖
| 类别 | 说明 |
|------|------|
| 框架 | **Qt 4.8**（C++11） |
| Qt 模块 | `core` `gui` `widgets` `printsupport` **`webkit`**（预览区使用 `QWebView` / QtWebKit） |
| Markdown 引擎 | **cmark-gfm**（GitHub Flavored Markdown），工程内捆绑静态库 `src/libcmark-gfm/libcmark-gfm.a` 与 `libcmark-gfm-extensions.a`，通过 `CMARK_GFM_STATIC_DEFINE` 以静态方式链接 |
| 配置持久化 | `QSettings`（INI 格式） |
| 国际化 | `tr()` + `QTextCodec`（强制 UTF-8） |
| 构建系统 | qmake（`project.pro`） |
| 资源 | `res/resources.qrc` |

> 最初计划采用QT5.12开发，后来发现其内置TextDocument对HTML渲染不理想，而WebEngine框架功能强大但过于沉重。  
> 随后考虑能使用Webkit的QT5.4来实现，开发一半发现QT5.4的菜单栏在Windows上显示异常。  
> 最后经过验证QT4.8可以完美避免以上问题。

### 1.3 第三方库说明
- **cmark-gfm**：用于把 Markdown 文本解析为 HTML。相比自研解析器，直接复用 CommonMark + GFM 扩展，保证解析的正确性与兼容性。渲染时按需挂载 GFM 扩展（表格、删除线、任务列表、自动链接、标签过滤），由 `ConfigManager` 的 parser 选项控制。

---

## 2. 系统架构

### 2.1 整体架构
```
┌─────────────────────────────────────────────────────────────┐
│                       MainWindow  (QMainWindow)             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              MenuBar / Toolbar / StatusBar            │  │
│  └───────────────────────────────────────────────────────┘  │
│  ┌─────────────────────┬─────────────────────────────────┐  │
│  │      CodeEditor     │        PreviewWidget            │  │
│  │  ┌───────────────┐  │  ┌───────────────────────────┐  │  │
│  │  │ QPlainTextEdit│  │  │ QWebView (QtWebKit)       │  │  │
│  │  │ +LineNumberArea│ │  │ + CSS Styling             │  │  │
│  │  │ +Highlighter  │  │  │ + Scroll Sync             │  │  │
│  │  └───────────────┘  │  └───────────────────────────┘  │  │
│  └─────────────────────┴─────────────────────────────────┘  │
│  QSplitter：左右分栏 / 上下分栏 / 仅编辑 / 仅预览                 │
└─────────────────────────────────────────────────────────────┘
         │ textChanged (防抖)          ▲ scrolled
         ▼                            │
┌────────────────┐          ┌────────────────────┐
│ MarkdownParser │          │   StyleSheetLoader │
│ (cmark-gfm)    │          │  (CSS 加载/校验)    │
└────────────────┘          └────────────────────┘
         │                            ▲
         └──────── ConfigManager ────┘  (QSettings 单例)
```

### 2.2 分层与数据流
1. **表现层**：`MainWindow` 负责窗口骨架（菜单、工具栏、状态栏）、`QSplitter` 布局与视图切换（左右 / 上下 / 仅编辑 / 仅预览 / 全屏）。
2. **编辑层**：`CodeEditor`（基于 `QPlainTextEdit`）负责源码编辑，配套 `LineNumberArea`（行号）与 `MarkdownHighlighter`（语法高亮）。
3. **预览层**：`PreviewWidget`（基于 `QWebView`）负责 HTML 渲染，委托 `StyleSheetLoader` 加载并校验 CSS。
4. **解析层**：`MarkdownParser` 封装 cmark-gfm，将 Markdown 转换为 HTML。
5. **配置层**：`ConfigManager` 单例，基于 `QSettings` 统一保存/读取编辑器、预览、编码、解析选项与窗口状态，变更时发出 `configurationChanged()`。

**核心数据流**：编辑区 `textChanged` → `MainWindow` 经定时器防抖（避免每次击键都重渲染）→ `MarkdownParser::parse()`（cmark-gfm）→ `PreviewWidget` 拼接完整 HTML 并套用 CSS → `QWebView::setHtml()` 异步渲染。编辑区与预览区通过滚动条比例做双向同步（`m_syncing` 标志防止递归）。

### 2.3 模块划分
| 模块 | 职责 | 关键类 |
|------|------|--------|
| 主窗口 | 布局、菜单/工具栏/状态栏、文件读写、视图切换、滚动同步 | `MainWindow` |
| 编辑区 | 文本编辑、行号、当前行高亮、空白显示、字体/Tab 宽度、自动换行 | `CodeEditor`, `LineNumberArea` |
| 语法高亮 | 按元素着色的 Markdown 高亮（配色来自配置） | `MarkdownHighlighter` |
| 预览区 | Markdown 渲染预览、CSS 样式、滚动同步、链接外链 | `PreviewWidget` |
| 解析器 | Markdown → HTML（cmark-gfm + GFM 扩展） | `MarkdownParser` |
| 配置管理 | 用户设置持久化（单例 + QSettings） | `ConfigManager` |
| 样式加载 | CSS 文件加载、花括号校验、默认样式回退 | `StyleSheetLoader` |
| 对话框 | 查找替换、偏好设置、插入链接/图片、关于 | `FindReplaceDialog`, `SettingsDialog`, `InsertDialog`, `AboutDialog` |

---

## 3. 概要设计

### 3.1 主窗口（MainWindow）
- 以 `QSplitter` 组织编辑区与预览区，支持**左右分栏、上下分栏、仅编辑、仅预览**四种视图，以及全屏模式；初次显示时强制 1:1 分隔。
- 菜单：文件（新建/打开/最近文件/保存/另存为/导出 HTML/导出 PDF/退出）、编辑（全选/查找/替换）、视图（全屏、自动换行、行号、空白字符、分隔方向、状态栏）、格式（加粗/斜体/下划线/删除线/标题/列表/引用/代码块/行内代码/链接/图片/ horizontal rule）、工具（偏好设置、解析选项开关、编码切换）、帮助（关于、Markdown 指南）。
- 状态栏显示光标位置与当前文件编码。
- 文件读写带**编码检测与保留**：加载时探测编码，保存时按原编码写回；可经菜单切换默认编码。
- 编辑区文本变更经防抖定时器后刷新预览；负责编辑区与预览区双向滚动同步。

### 3.2 编辑区（CodeEditor）
- 继承 `QPlainTextEdit`，提供 Markdown 源码编辑。
- `LineNumberArea`：作为左 margin 子控件绘制行号，随块变化与滚动同步。
- 当前行高亮（可配置颜色）。
- 空白字符显示（`QTextOption::ShowTabsAndSpaces` 等）。
- 自定义字体、Tab 宽度（像素换算）、自动换行开关。

### 3.3 语法高亮（MarkdownHighlighter）
- 继承 `QSyntaxHighlighter`，按块着色，仅改变前景色。
- 覆盖标题、代码块（含跨行围栏代码块状态机）、引用、列表、水平线、粗体、斜体、删除线、链接、表格等元素。
- 各元素颜色由 `ConfigManager` 提供，可在偏好设置中调整。
- 通过 `setDocument()` 挂载/卸载实现启停。

### 3.4 预览区（PreviewWidget）
- 继承 `QWebView`（QtWebKit），渲染解析后的完整 HTML。
- 套用 CSS 样式（本地文件或内置默认样式）；支持设置 `baseUrl` 以正确解析相对路径资源。
- 链接点击委托系统浏览器（`QDesktopServices`）打开。
- 双向比例滚动同步：编辑器滚动条变化按比例驱动预览；预览侧以定时器轮询网页滚动位置变化并发出 `scrolled`，反向驱动编辑器（`m_syncing` 防递归；`setHtml()` 异步加载完成后重新应用待定滚动比例）。

### 3.5 解析器（MarkdownParser）
- 封装 cmark-gfm：输入 Markdown（转 UTF-8）→ `cmark_parser_new` → 按需挂载 GFM 扩展 → `cmark_render_html` → 输出 HTML 字符串（UTF-8）。
- 暴露 `parserOptions()` 列表（表格、删除线、自动链接、标签过滤、任务列表），各开关由 `ConfigManager` 的 parser 选项决定；默认开启表格、删除线、任务列表。

### 3.6 配置管理（ConfigManager）
- 单例（构造函数私有，禁用拷贝），基于 `QSettings`（INI）读写，变更时 `emit configurationChanged()`。
- 管理内容：
  - **编辑器**：字体、自动换行、行号、空白字符、语法高亮、Tab 宽度；
  - **语法高亮配色**：当前行、标题、代码、引用、列表、水平线、粗体、斜体、删除线、链接、表格；
  - **编码**：默认编码；
  - **解析选项**：各 GFM 扩展开关；
  - **预览**：浏览器字体族（standard/serif/sans-serif/monospace）、预览样式文件；
  - **窗口状态**：geometry / state；
  - **最近文件**：去重的最近打开列表。

### 3.7 样式加载（StyleSheetLoader）
- 读取本地 CSS 文件（UTF-8），校验花括号配平；失败时回退到内置 `getDefaultCSS()`。
- 内置默认样式为 GitHub 风格（标题下边框、代码块底色、`blockquote` 左侧线、表格边框等）。

### 3.8 对话框
- `FindReplaceDialog`：模态查找/替换，支持大小写敏感、整词匹配、全部替换。
- `SettingsDialog`：偏好设置（编辑器 + 预览 + 高亮配色 + 编码），绑定 `ConfigManager`。
- `InsertDialog`：插入链接 / 图片的输入提示框（`kind` 区分模式）。
- `AboutDialog`：显示版本、Qt 版本与许可证信息。

---

## 4. 文件结构

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
│   ├── aboutdialog.h / aboutdialog.cpp
│   └── libcmark-gfm/            # 捆绑的 cmark-gfm 静态库与头文件
│       ├── cmark-gfm.h / cmark-gfm-core-extensions.h / ...
│       ├── libcmark-gfm.a
│       └── libcmark-gfm-extensions.a
├── res/
│   ├── resources.qrc           # 内置 styles/default.css, styles/app.qss
│   └── app.rc                  # Windows EXE 图标
├── styles/
│   ├── default.css             # 预览默认样式（GitHub 风格）
│   └── github.css              # GitHub 风格预览样式
├── project.pro                # qmake 工程文件
└── readme.md
```

---

## 5. 构建与运行

```bash
# 修改build.bat中定义QT路径
build.bat
# 生成 MarkEdit.exe
```

> 说明：预览区依赖 QtWebKit（`QWebView`），需使用包含 WebKit 模块的构建环境。cmark-gfm 以静态库形式随工程提供，无需额外安装。
