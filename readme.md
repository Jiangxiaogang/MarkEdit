# MarkEdit

基于 Qt 的跨平台 Markdown 编辑器，采用左右分栏布局：左侧为 Markdown 源码编辑区，右侧为实时渲染预览区。编辑器支持行号、空白字符显示、可配置配色的语法高亮、字体与 Tab 宽度设置、编码选择与转换；预览区基于 QtWebKit 渲染，样式可由本地 CSS 自定义，并支持编辑区与预览区双向滚动同步。


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
- **cmark-gfm**：用于把 Markdown 文本解析为 HTML。相比自研解析器，直接复用 CommonMark + GFM 扩展，保证解析的正确性与兼容性。渲染时按需挂载 GFM 扩展（表格、删除线、任务列表、自动链接、标签过滤）与渲染选项（保留单行换行／硬换行），由 `ConfigManager` 的 parser 选项控制。

---

## 2. 系统架构

### 2.1 整体架构
```
┌─────────────────────────────────────────────────────────────┐
│                       MainWindow  (QMainWindow)             │
│  ┌───────────────────────────────────────────────────────┐  │
│  │              MenuBar / StatusBar                       │  │
│  └───────────────────────────────────────────────────────┘  │
│  ┌─────────────────────┬─────────────────────────────────┐  │
│  │      CodeEditor     │        PreviewWidget            │  │
│  │  ┌───────────────┐  │  ┌───────────────────────────┐  │  │
│  │  │ QPlainTextEdit│  │  │ QWebView (QtWebKit)       │  │  │
│  │  │ +LineNumberArea│ │  │ + CSS Styling             │  │  │
│  │  │ +Highlighter  │  │  │ + Scroll Sync             │  │  │
│  │  └───────────────┘  │  └───────────────────────────┘  │  │
│  └─────────────────────┴─────────────────────────────────┘  │
│  QSplitter：左右分栏 / 仅编辑 / 仅预览                            │
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
1. **表现层**：`MainWindow` 负责窗口骨架（菜单、状态栏）、`QSplitter` 布局与视图切换（左右分栏 / 仅编辑 / 仅预览 / 全屏）。
2. **编辑层**：`CodeEditor`（基于 `QPlainTextEdit`）负责源码编辑，配套 `LineNumberArea`（行号）与 `MarkdownHighlighter`（语法高亮）。
3. **预览层**：`PreviewWidget`（基于 `QWebView`）负责 HTML 渲染，委托 `StyleSheetLoader` 加载并校验 CSS。
4. **解析层**：`MarkdownParser` 封装 cmark-gfm，将 Markdown 转换为 HTML。
5. **配置层**：`ConfigManager` 单例，基于 `QSettings` 统一保存/读取编辑器、预览、编码、解析选项与窗口状态，确认偏好设置后发出 `configurationChanged()`。

**核心数据流**：编辑区 `textChanged` → `MainWindow` 经定时器防抖（避免每次击键都重渲染）→ `MarkdownParser::parse()`（cmark-gfm）→ `PreviewWidget` 拼接完整 HTML 并套用 CSS → `QWebView::setHtml()` 异步渲染。编辑区与预览区通过滚动条比例做双向同步（`m_syncing` 标志防止递归）。

### 2.3 模块划分
| 模块 | 职责 | 关键类 |
|------|------|--------|
| 主窗口 | 布局、菜单/状态栏、文件读写、视图切换、滚动同步 | `MainWindow` |
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
- 以 `QSplitter` 组织编辑区与预览区，支持**左右分栏、仅编辑、仅预览**三种视图，以及全屏模式；初次显示时强制 1:1 分隔。
- 菜单：文件（新建/打开/最近文件/保存/另存为/导出 HTML/导出 PDF/退出）、编辑（全选/查找/替换）、视图（全屏、自动换行、行号、空白字符、左右分栏/仅编辑/仅预览、状态栏）、格式（加粗/斜体/下划线/删除线/标题/列表/引用/代码块/行内代码/链接/图片/ horizontal rule）、工具（偏好设置、解析选项开关、编码切换）、帮助（关于）。
- 状态栏显示光标位置与当前文件编码。
- 文件读写保留编码：加载时按当前默认编码读取（不做自动探测），保存时按原编码写回；可经菜单切换默认编码。
- 编辑区文本变更经防抖定时器后刷新预览；负责编辑区与预览区双向滚动同步。

### 3.2 编辑区（CodeEditor）
- 继承 `QPlainTextEdit`，提供 Markdown 源码编辑。
- `LineNumberArea`：作为左 margin 子控件绘制行号，随块变化与滚动同步。
- 当前行高亮（可配置颜色）。
- 空白字符显示（`QTextOption::ShowTabsAndSpaces` 等）。
- 自定义字体、Tab 宽度（像素换算）、自动换行开关。

### 3.3 语法高亮（MarkdownHighlighter）
- 继承 `QSyntaxHighlighter`，按块着色，仅改变前景色。
- 覆盖标题、代码块（含跨行围栏代码块状态机）、引用、列表、水平线、粗体、删除线、链接等元素。
- 各元素颜色由 `ConfigManager` 提供；偏好设置中仅可调整当前行底色，其余颜色可通过 `config.ini` 中的 `editor/*_color` 项修改。
- 通过 `setDocument()` 挂载/卸载实现启停。

### 3.4 预览区（PreviewWidget）
- 继承 `QWebView`（QtWebKit），渲染解析后的完整 HTML。
- 套用 CSS 样式（本地文件或内置默认样式）；支持设置 `baseUrl` 以正确解析相对路径资源。
- `refresh()` 在 `setHtml()` 前记录当前滚动比例，加载完成后恢复，因此编辑内容导致重渲染时预览区不会跳回顶部（与滚动同步开关无关）。
- 链接点击委托系统浏览器（`QDesktopServices`）打开。
- 支持 `Ctrl+C` 复制选中文字与 `Ctrl+A` 全选（`keyPressEvent` 映射到 `QWebPage::Copy` / `SelectAll`；编辑菜单不注册窗口级快捷键，避免抢占预览区按键事件）。
- 双向比例滚动同步：编辑器滚动条变化按比例驱动预览；预览侧以定时器轮询网页滚动位置变化并发出 `scrolled`，反向驱动编辑器（`m_syncing` 防递归；`setHtml()` 异步加载完成后重新应用待定滚动比例）。

### 3.5 解析器（MarkdownParser）
- 无状态静态工具类：`parse()` 封装 cmark-gfm，输入 Markdown（转 UTF-8）→ `cmark_parser_new` → 按需挂载 GFM 扩展 → `cmark_render_html` → 输出 HTML 字符串（UTF-8）；解析器与语法树均为函数内局部对象，用完即释放。
- `parserOptions()` 暴露选项列表（表格、删除线、自动链接、标签过滤、任务列表、保留单行换行），各开关由 `ConfigManager` 的 parser 选项决定；默认开启表格、删除线、任务列表。GFM 扩展通过 `cmark_parser_attach_syntax_extension()` 挂载，渲染类选项（如 `CMARK_OPT_HARDBREAKS`）通过 `renderFlags` 汇总后传给 `cmark_render_html()`。

### 3.6 配置管理（ConfigManager）
- 单例（构造函数私有，禁用拷贝），基于 `QSettings`（INI）读写；`notifyChanged()` 显式发出 `configurationChanged()`，由偏好设置的实时修改和需要刷新的场景调用，`MainWindow` 与 `PreviewWidget` 均监听该信号。
- 管理内容：
  - **编辑器**：字体、自动换行、行号、空白字符、语法高亮、Tab 宽度；
  - **语法高亮配色**：当前行、标题、代码、引用、列表、水平线、粗体、删除线、链接；
  - **编码**：默认编码；
  - **解析选项**：各 GFM 扩展开关；
  - **预览**：标准字体与标准字号、等宽字体与等宽字号、预览样式文件；
  - **窗口状态**：geometry / state；
  - **最近文件**：去重的最近打开列表。

### 3.7 样式加载（StyleSheetLoader）
- 无状态工具类，全部为静态方法，不再依赖 `QObject` / 信号。
- `load(path, errorMessage)` 读取本地 CSS 文件（UTF-8）；空路径表示“内置样式”，不算错误。文件不存在、无法打开或校验失败时回退内置样式，并通过可选的 `errorMessage` 返回原因。
- `validate(css, errorMessage)` 校验花括号配平，跳过注释与引号字符串，报告出错行号。
- `defaultStyle()` 读取并缓存内置样式；样式来自 `styles/default.css`（经 `res/resources.qrc` 编译为 `:/styles/default.css`），不再另存一份硬编码副本。

### 3.8 对话框
- `FindReplaceDialog`：模态查找/替换，支持大小写敏感、整词匹配、全部替换。
- `SettingsDialog`：偏好设置（编辑器字体/字号/Tab 宽度/当前行底色，预览样式文件、标准/等宽字体与字号）。控件值变化时立即写入配置、保存并发出 `configurationChanged()`，编辑区与预览区实时更新；底部仅保留“确定”按钮，无“取消”回滚。
- `InsertDialog`：插入链接 / 图片的输入提示框。
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
│   ├── resources.qrc           # 将 styles/default.css 编译为内置资源
│   └── app.rc                  # Windows EXE 图标
├── styles/
│   ├── default.css             # 预览默认样式（内置资源，GitHub 风格）
│   └── github.css              # 可选：放在 exe 同目录 styles/ 下供偏好设置选择
├── project.pro                # qmake 工程文件
└── readme.md
```

---

## 5. 构建与运行

```bash
# 修改build.bat中定义QT路径
build.bat
# 生成 MarkEdit.exe

# 直接运行 release 目录下的程序（自动配置 Qt 运行时路径）
run.bat
```

> 说明：预览区依赖 QtWebKit（`QWebView`），需使用包含 WebKit 模块的构建环境。cmark-gfm 以静态库形式随工程提供，无需额外安装。

---

## 6. 与主流 Markdown 编辑器对比

### 6.1 主流技术路线

市面上的 Markdown 编辑器大体分四条技术路线，各有典型取舍：

| 路线 | 代表产品 | 优点 | 代价 |
|------|----------|------|------|
| Electron / Chromium 全栈 | Typora、Obsidian、VS Code、Mark Text、Joplin | 完整浏览器内核，CSS3 / JS / MathJax / Mermaid 渲染无压力，前端生态成熟 | 安装包与内存达数百 MB 量级，多进程、启动慢、资源占用高 |
| 原生框架 + 嵌入浏览器控件 | MarkdownPad（IE/MSHTML）、Markdown Monster（WebView2）、Haroopad（NW.js）、**本项目（QtWebKit）** | 界面原生轻快，渲染能力接近浏览器 | 渲染能力取决于嵌入内核的新旧，内核老旧则 CSS/JS 支持受限 |
| 纯原生控件渲染 | 早期 Qt TextDocument、部分 GTK 编辑器 | 最轻量 | HTML/CSS 渲染能力弱，复杂排版（表格、代码块、引用）效果差 |
| 纯 Web 应用 | StackEdit、Dillinger 等在线编辑器 | 免安装、天然跨平台 | 依赖浏览器沙箱与网络，本地文件读写受限 |

### 6.2 常见编辑器的技术栈与主要缺点

| 编辑器 | 主要技术 | 主要缺点 |
|--------|----------|----------|
| Typora | Electron + 自研混合所见即所得渲染 | 闭源收费；Electron 体积/内存大；无插件体系，扩展仅限 CSS 主题 |
| VS Code（Markdown 支持） | Electron + Monaco 编辑器 + markdown-it 解析 | 通用 IDE 而非专用写作工具，配置繁琐；预览与编辑滚动同步较弱；资源占用高 |
| Obsidian | Electron + CodeMirror 6 + 本地库/双链体系 | 核心体验围绕"库/双链"设计，单纯写作场景偏重；闭源（个人免费 + 订阅服务）；体积/内存大 |
| Mark Text | Electron + muya（自研 WYSIWYG 引擎） | 项目长期停滞、Bug 较多、功能欠完整；Electron 体积大 |
| Joplin | Electron + CodeMirror + 同步服务 | 定位是笔记库/同步工具而非专注写作；体积大 |
| MarkdownPad | WPF (.NET) + IE（MSHTML）渲染 | 仅 Windows；IE 内核 CSS/HTML5 支持差；2013 年后停更；免费版功能受限 |
| Markdown Monster | WPF (.NET) + WebView2 + Markdig 解析 | 收费；仅 Windows；依赖 .NET 运行时 |
| Haroopad | NW.js + 浏览器内核渲染 + MathJax | 2016 年后停更；体积大；社区失活 |
| Mou | macOS 原生 + WebKit | 仅 macOS 且停更 |
| QOwnNotes / Ghostwriter | 原生 Qt（C++） | 轻量但功能与插件生态有限，界面朴素，多平台打磨不足 |

> 规律：重型编辑器普遍押注 Electron/Chromium 换取渲染能力与生态，代价是体积、内存与启动速度；轻量路线则必须在渲染引擎上做取舍——这正是本项目选型时权衡的核心矛盾（见 §1.2 中 Qt5.12 → Qt5.4 → Qt4.8 的选型过程）。

### 6.3 本项目的优点与特点

**架构与选型**

- **轻量高效**：纯 C++ 原生程序，单进程运行，启动快、内存占用低（数十 MB 量级），无 Electron 数百 MB 的安装体积与内存开销；cmark-gfm 以静态库随工程链接，无额外运行时依赖。
- **渲染质量与轻量的平衡**：预览基于 QtWebKit（`QWebView`），同时规避了 Qt 自带 TextDocument 渲染 HTML 不理想的缺陷与 WebEngine/Chromium 内核过于沉重的问题；"原生控件 + 嵌入式渲染内核"与 MarkdownPad 同类，但内核更新、体积更小。
- **解析正确性有保障**：直接复用 GitHub 官方 C 实现 cmark-gfm，严格遵循 CommonMark + GFM 规范，表格、删除线、任务列表等复杂语法的解析结果与 GitHub 一致，规避了自研/正则解析器常见的边界错误；GFM 扩展与渲染选项可按需开关（表格、删除线、自动链接、标签过滤、任务列表、保留单行换行）。

**编辑体验**

- 行号、空白字符显示、当前行高亮、自动换行、字体与 Tab 宽度均可调；
- 十余种 Markdown 元素（标题、代码、引用、列表、水平线、粗体、删除线、链接等）的语法高亮颜色由 `config.ini` 配置；偏好设置中可调整当前行底色与编辑器字体；
- 编辑区与预览区**双向比例滚动同步**（编辑器滚动驱动 + 预览定时轮询驱动，`m_syncing` 防递归）；
- 预览刷新带 500ms 防抖，连续输入不会反复触发整页重渲染；
- 预览区支持 `Ctrl+C` / `Ctrl+A` 复制与全选，链接点击委托系统浏览器打开。

**中文与文件场景**

- 编码保留：加载时按当前默认编码读取（不做自动探测），保存时按原编码写回；支持 System / UTF-8 / UTF-16 / UTF-32 手动切换；
- 文件被外部程序修改时自动检测：窗口重新激活时比较修改时间与文件大小（无需轮询或文件监视器），提示是否重新加载；自身保存不会误触发提示；
- 查找/替换、最近文件、导出 HTML、打印/导出 PDF 等写作配套功能齐全。

**配置与定制**

- 配置以 `QSettings` INI 纯文本持久化，无隐藏状态，可直接编辑；
- 预览样式即本地 CSS 文件：加载校验（花括号配平）、失败自动回退内置 GitHub 风格样式，主题定制零门槛；
- MIT 开源，模块分层清晰（编辑 / 预览 / 解析 / 配置 / 样式加载各司其职），便于二次开发。

