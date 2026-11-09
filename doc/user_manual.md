# MarkEdit — User Manual

MarkEdit is a cross-platform Markdown editor built with Qt. It provides a
split-pane interface: a Markdown source editor on the left and a live HTML
preview on the right.

## Getting Started

1. Launch **MarkEdit** (optionally pass a file path on the command line).
2. Type Markdown in the left pane — the right pane updates automatically
   (500 ms debounce).
3. Use the **File** menu to create, open, save and export documents.

## Menu Reference

### File
- **New** (`Ctrl+N`) – new empty document.
- **Open…** (`Ctrl+O`) – open a Markdown / text file.
- **Open Recent** – reopen one of the last 10 files.
- **Save** (`Ctrl+S`) – save the current document.
- **Save As…** (`Ctrl+Shift+S`) – save under a new name.
- **Export as HTML** (`Ctrl+E`) – write a standalone styled HTML file.
- **Export as PDF** (`Ctrl+P`) – print the preview to a PDF file.
- **Exit** (`Alt+F4`).

### Edit
- **Undo** / **Redo** (`Ctrl+Z` / `Ctrl+Y`).
- **Cut** / **Copy** / **Paste** (`Ctrl+X` / `Ctrl+C` / `Ctrl+V`);
  copy uses the editor's native shortcut.
- **Select All** (`Ctrl+A`, native shortcut).
- **Find…** / **Replace…** (`Ctrl+F` / `Ctrl+H`).

### View
- **Line Wrap** – toggle soft wrapping in the editor.
- **Show Line Numbers** – toggle the gutter.
- **Show Whitespace** – reveal tabs and spaces.
- **Edit Preview** (default) / **Editor Only** / **Preview Only**.
- **Full Screen** (`F11`).
- **Status Bar** – show/hide the status bar.

> Only a left/right split is available; there is no top/bottom split.

### Format
- **Bold** (`Ctrl+B`), **Italic** (`Ctrl+I`), **Underline** (`Ctrl+U`),
  **Strikethrough** (`Ctrl+T`).
- **Heading 1/2/3** (`Ctrl+1/2/3`).
- **Unordered List** (`Ctrl+L`), **Ordered List** (`Ctrl+Shift+L`),
  **Block Quote** (`Ctrl+Q`), **Code Block** (`Ctrl+K`), **Inline Code** (`Ctrl+'`).
- **Insert Link…**, **Insert Image…**, **Insert Horizontal Rule**.

### Tools
- **Encoding** – choose System / UTF-8 / UTF-16 / UTF-32; the current file is
  reloaded with the selected encoding.
- **Syntax Extensions** – toggle GFM extensions: table, strikethrough,
  autolink, tagfilter, tasklist.
- **Preferences…** – editor font, font size, tab width, current-line colour,
  preview stylesheet and preview font families.

### Help
- **About** (`F1`) – version and license information.

## Customising the Preview

The preview pane renders your Markdown with a CSS stylesheet:

- The built-in default stylesheet is compiled from `styles/default.css`
  into the executable (`:/styles/default.css`).
- **Tools → Preferences… → Stylesheet file** lists `*.css` files found in a
  `styles/` directory next to the executable, and falls back to the built-in
  style when the selection is empty.
- Invalid CSS (unbalanced braces) or an unreadable file falls back to the
  built-in style.

## Encoding

- Files are loaded with the current default encoding (no automatic
  detection) and saved with the same encoding.
- Use **Tools → Encoding** to switch between System / UTF-8 / UTF-16 /
  UTF-32; the current file is reloaded immediately.

## External File Changes

When the window regains focus, MarkEdit compares the file's modification
time and size with the last known snapshot and offers to reload if another
program changed it. A file saved by MarkEdit itself does not trigger this
prompt.

## Configuration

Preferences are stored in `config.ini` next to the executable
(`QSettings`, INI format) and are restored on the next launch, including
window geometry, recent files, editor colours, parser options and the chosen
stylesheet.
