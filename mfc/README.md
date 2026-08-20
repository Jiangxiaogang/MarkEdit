# MFC Markdown Editor - VS2017 工程

## 项目说明

这是一个基于 MFC 的 Markdown 编辑器，使用 VS2017 (v141 工具集) 构建。

### 功能特点

- **左侧编辑，右侧实时预览**：界面分为左右两部分，左边是 Markdown 编辑区，右边是 HTML 预览区
- **MSHTML 内核预览**：使用 CHtmlView 和 MSHTML 引擎渲染预览，不使用第三方库
- **纯原生实现**：仅使用 MFC 和 Windows SDK，无任何第三方依赖

### 菜单结构

- **文件 (F)**: 新建、打开、保存、另存为、退出
- **编辑 (E)**: 撤销、重做、剪切、复制、粘贴、全选
- **视图 (V)**: 全屏、刷新预览
- **格式 (O)**: 粗体、斜体、一级标题、二级标题、三级标题、代码、列表
- **工具 (T)**: 选项
- **帮助 (H)**: 关于

### 支持的 Markdown 语法

- 标题 (# 到 ######)
- 粗体 (**text**)
- 斜体 (*text*)
- 行内代码 (`code`)
- 代码块 (```)
- 链接 [text](url)
- 图片 ![alt](url)
- 无序列表 (- item)
- 有序列表 (1. item)
- 引用 (> quote)
- 水平线 (---)

### 使用方法

1. 在 Visual Studio 2017 中打开 `MFCMarkdownEditor.vcxproj`
2. 编译生成可执行文件
3. 运行程序即可开始编辑 Markdown 文档

### 注意事项

- 需要在 Windows 环境下使用 Visual Studio 2017 编译
- 需要安装 MFC 库支持
- res/MFCMarkdownEditor.ico 需要一个有效的图标文件（可从其他 MFC 项目复制或自行创建）

### 文件结构

```
mfc/
├── MFCMarkdownEditor.vcxproj      # VS2017 项目文件
├── MFCMarkdownEditor.vcxproj.filters
├── framework.h                     # MFC 框架头文件
├── targetver.h                     # Windows 版本定义
├── pch.h                           # 预编译头文件
├── pch.cpp                         # 预编译源文件
├── Resource.h                      # 资源定义
├── MFCMarkdownEditor.h             # 应用程序类头文件
├── MFCMarkdownEditor.cpp           # 应用程序类实现
├── MFCMarkdownEditorDlg.h          # 主对话框头文件
├── MFCMarkdownEditorDlg.cpp        # 主对话框实现
├── MarkdownParser.h                # Markdown 解析器头文件
├── MarkdownParser.cpp              # Markdown 解析器实现
├── MFCMarkdownEditor.rc            # 资源脚本
└── res/
    ├── MFCMarkdownEditor.ico       # 应用程序图标（需自行提供）
    └── MFCMarkdownEditor.rc2       # 资源扩展文件
```
