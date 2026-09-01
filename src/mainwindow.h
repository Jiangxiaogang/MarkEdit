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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &filePath = QString(), QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private:
    // UI construction (called directly, not via signals)
    void initUI();
    void initMenuBar();
    void initStatusBar();
    void initConnections();
    void loadSettings();
    void saveSettings();

private slots:
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
    void toggleLineWrap(bool checked);
    void toggleLineNumber(bool checked);
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
    void openPreferences();
    void onParserOptionToggled(bool checked);
    void onEncodingTriggered(QAction* action);
    void onEncodingSelected(const QString &codecName);
    void updateEncodingMenu();

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
    void previewCopy();
    void previewSelectAll();

    // Helpers
    void updateRecentMenu();
    void clearRecentFilesAction();
    void wrapSelection(const QString &before, const QString &after);
    void applyLinePrefix(const QString &prefix, bool toggle = true);
    void insertAtCursor(const QString &text);
    bool maybeSave();
    void setCurrentFile(const QString &path);
    void applyConfigToUi();

    // File encoding: detect on load and preserve on save
    bool loadFile(const QString &path);
    bool loadFileWithEncoding(const QString &path, const QString &encoding);

private:
    CodeEditor *m_editor;
    PreviewWidget *m_preview;
    QSplitter *m_splitter;
    ConfigManager *m_config;
    StyleSheetLoader *m_styleLoader;

    QTimer *m_previewTimer;

    QString m_currentFile;
    QString m_fileEncoding;
    bool m_syncing;
    bool m_splitSet;

    // Menu actions kept for enable/disable & state tracking
    QAction *m_saveAction;
    QMenu *m_recentMenu;
    QMenu *m_encodingMenu;
    QList<QAction *> m_encodingActions;
    QList<QAction *> m_parserOptionActions;
    QAction *m_lineWrapAction;
    QAction *m_lineNumberAction;
    QAction *m_whitespaceAction;
    QAction *m_statusBarAction;
    QAction *m_fullScreenAction;
    QAction *m_verticalSplitAction;
    QAction *m_editorOnlyAction;
    QAction *m_previewOnlyAction;

    // Status bar widgets
    QLabel *m_statusCursor;
    QLabel *m_statusEncoding;
};

#endif // MAINWINDOW_H
