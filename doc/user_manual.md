# MarkEdit — User Manual

MarkEdit is a cross-platform Markdown editor built with Qt. It provides a
split-pane interface: a Markdown source editor on the left and a live HTML
preview on the right.

## Getting Started

1. Launch **MarkEdit**.
2. Type Markdown in the left pane — the right pane updates automatically.
3. Use the **File** menu to create, open, save and export documents.

## Menu Reference

### File
- **New** (`Ctrl+N`) – new empty document.
- **Open…** (`Ctrl+O`) – open a `.md` / `.markdown` / `.txt` file.
- **Open Recent** – reopen one of the last 10 files.
- **Save** (`Ctrl+S`) – save the current document.
- **Save As…** (`Ctrl+Shift+S`) – save under a new name.
- **Export as HTML** (`Ctrl+E`) – write a standalone styled HTML file.
- **Export as PDF** (`Ctrl+P`) – print the preview to a PDF file.
- **Exit** (`Alt+F4`).

### Edit
- **Undo** / **Redo** (`Ctrl+Z` / `Ctrl+Y`).
- **Cut** / **Copy** / **Paste** (`Ctrl+X` / `C` / `V`).
- **Select All** (`Ctrl+A`).
- **Find…** / **Replace…** (`Ctrl+F` / `Ctrl+H`).

### View
- **Full Screen** (`F11`).
- **Show Line Numbers** – toggle the gutter.
- **Show Whitespace** (`Ctrl+Shift+B`) – reveal tabs, spaces and paragraph marks.
- **Vertical Split** (default), **Editor Only**, **Preview Only**.
- **Status Bar** – show/hide the status bar.

### Format
- **Bold** (`Ctrl+B`), **Italic** (`Ctrl+I`), **Underline** (`Ctrl+U`),
  **Strikethrough** (`Ctrl+T`).
- **Heading 1/2/3** (`Ctrl+1/2/3`).
- **Unordered List** (`Ctrl+L`), **Ordered List** (`Ctrl+Shift+L`),
  **Block Quote** (`Ctrl+Q`), **Code Block** (`Ctrl+K`), **Inline Code** (`Ctrl+'`).
- **Insert Link…**, **Insert Image…**, **Insert Horizontal Rule**.

### Tools
- **Select CSS Style…** – load a custom stylesheet for the preview.
- **Reset CSS Style** – restore the built-in default stylesheet.
- **Preferences…** (`Ctrl+,`) – editor font, whitespace, tab width,
  CSS path and preview behaviour.

### Help
- **About** – version and license information.
- **Markdown Syntax Guide** (`F1`).

## Customising the Preview

The preview pane renders your Markdown with a CSS stylesheet:

- The default stylesheet is bundled with the application.
- Choose **Tools → Select CSS Style…** to load your own `*.css` file.
- A dark theme (`dark.css`) ships in `resources/styles/` and can be selected
  the same way.

## Configuration

Preferences are stored in a platform-specific location
(`QSettings`, INI format) under the `MarkEdit` organisation and are restored
on the next launch, including window geometry, recent files and the chosen
stylesheet.
