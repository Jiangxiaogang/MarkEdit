#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CodeEditor;
class PreviewWidget;
class ConfigManager;
class StyleSheetLoader;
class QSplitter;
class QAction;
class QLabel;
class QTimer;

/**
 * @brief Main application window.
 *
 * Owns the editor / preview splitter, the menu bar, toolbar and status bar,
 * and wires together all file, edit, view, format, tools and help actions
 * described in menu.md.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private:
    // UI construction
    void initUI();
    void initMenus();
    void initStatusBar();
    void initConnections();
    void loadSettings();
    void saveSettings();

    // File
    void onNewFile();
    void onOpenFile();
    void onOpenRecent();
    void onSaveFile();
    void onSaveAs();
    bool saveFile(const QString &path);
    void onExportHtml();
    void onExportPdf();
    void onExit();

    // Edit
    void onSelectAll();
    void onFind();
    void onReplace();

    // View
    void toggleFullScreen();
    void toggleLineNumbers(bool checked);
    void toggleWhitespace(bool checked);
    void setVerticalSplit();
    void showEditorOnly();
    void showPreviewOnly();
    void toggleStatusBar(bool checked);

    // Format
    void formatBold();
    void formatItalic();
    void formatUnderline();
    void formatStrikethrough();
    void heading(int level);
    void bulletList();
    void orderedList();
    void blockQuote();
    void codeBlock();
    void inlineCode();
    void insertLink();
    void insertImage();
    void insertHorizontalRule();

    // Tools
    void selectCss();
    void resetCss();
    void openPreferences();

    // Help
    void about();
    void markdownGuide();

    // Preview / editor plumbing
    void onTextChanged();
    void updatePreview();
    void updateStatus();
    void syncScrollFromEditor(int value);
    void syncScrollFromPreview(int value);

    // Context menus
    void editorContextMenu(const QPoint &pos);
    void previewContextMenu(const QPoint &pos);

    // Helpers
    void updateRecentMenu();
    void wrapSelection(const QString &before, const QString &after);
    void applyLinePrefix(const QString &prefix, bool toggle = true);
    void insertAtCursor(const QString &text);
    bool maybeSave();
    void setCurrentFile(const QString &path);
    void applyConfigToUi();

    CodeEditor *m_editor;
    PreviewWidget *m_preview;
    QSplitter *m_splitter;
    ConfigManager *m_config;
    StyleSheetLoader *m_styleLoader;

    QTimer *m_previewTimer;

    QString m_currentFile;
    bool m_syncing;
    bool m_splitSet;

    // Menu actions kept for enable/disable & state tracking
    QAction *m_saveAction;
    QMenu *m_recentMenu;
    QAction *m_lineNumbersAction;
    QAction *m_whitespaceAction;
    QAction *m_statusBarAction;
    QAction *m_fullScreenAction;
    QAction *m_verticalSplitAction;
    QAction *m_editorOnlyAction;
    QAction *m_previewOnlyAction;

    // Status bar widgets
    QLabel *m_statusCursor;
    QLabel *m_statusWords;
};

#endif // MAINWINDOW_H
