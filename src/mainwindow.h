#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QToolBar>
#include <QFontDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>

#include "editor/codeeditor.h"
#include "preview/previewwidget.h"
#include "parser/markdownparser.h"
#include "config/configmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    // 文件菜单
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exportToHtml();
    void exportToPdf();
    void printFile();
    void exitApp();

    // 编辑菜单
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void findReplace();
    void goToLine();

    // 视图菜单
    void toggleLineNumbers(bool checked);
    void toggleWhitespace(bool checked);
    void toggleFullScreen(bool checked);
    void togglePreviewPane(bool checked);
    void splitHorizontally(bool checked);
    void splitVertically(bool checked);
    void resetLayout();
    void zoomIn();
    void zoomOut();
    void resetZoom();

    // 格式菜单
    void formatBold();
    void formatItalic();
    void formatUnderline();
    void formatStrikethrough();
    void formatInlineCode();
    void formatHeader1();
    void formatHeader2();
    void formatHeader3();
    void formatBulletList();
    void formatNumberedList();
    void formatBlockquote();
    void formatCodeBlock();
    void formatLink();
    void formatImage();
    void formatHorizontalRule();

    // 工具菜单
    void showSettings();
    void loadCssFile();
    void resetCss();

    // 帮助菜单
    void aboutApp();
    void aboutQt();

    // 其他槽函数
    void onTextChanged();
    void updatePreviewFromTimer();
    void updateWindowTitle();
    void updateStatusBar();

private:
    void initUI();
    void initMenuBar();
    void initConnections();
    void loadSettings();
    void saveSettings();
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createFormatMenu();
    void createToolsMenu();
    void createHelpMenu();
    void updateMenusState();
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    
    // 核心组件
    CodeEditor *m_editor;
    PreviewWidget *m_preview;
    QSplitter *m_splitter;
    
    // 解析器和配置
    MarkdownParser *m_parser;
    ConfigManager *m_config;
    
    // 菜单
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_formatMenu;
    QMenu *m_toolsMenu;
    QMenu *m_helpMenu;
    
    // 动作
    QAction *m_actNew;
    QAction *m_actOpen;
    QAction *m_actSave;
    QAction *m_actSaveAs;
    QAction *m_actExportHtml;
    QAction *m_actExportPdf;
    QAction *m_actPrint;
    QAction *m_actExit;
    
    QAction *m_actUndo;
    QAction *m_actRedo;
    QAction *m_actCut;
    QAction *m_actCopy;
    QAction *m_actPaste;
    QAction *m_actSelectAll;
    QAction *m_actFindReplace;
    QAction *m_actGoToLine;
    
    QAction *m_actLineNumbers;
    QAction *m_actWhitespace;
    QAction *m_actFullScreen;
    QAction *m_actPreviewPane;
    QAction *m_actSplitH;
    QAction *m_actSplitV;
    QAction *m_actResetLayout;
    QAction *m_actZoomIn;
    QAction *m_actZoomOut;
    QAction *m_actResetZoom;
    
    QAction *m_actBold;
    QAction *m_actItalic;
    QAction *m_actUnderline;
    QAction *m_actStrike;
    QAction *m_actInlineCode;
    QAction *m_actH1;
    QAction *m_actH2;
    QAction *m_actH3;
    QAction *m_actBulletList;
    QAction *m_actNumberedList;
    QAction *m_actBlockquote;
    QAction *m_actCodeBlock;
    QAction *m_actLink;
    QAction *m_actImage;
    QAction *m_actHr;
    
    QAction *m_actSettings;
    QAction *m_actLoadCss;
    QAction *m_actResetCss;
    
    QAction *m_actAbout;
    QAction *m_actAboutQt;
    
    // 状态栏组件
    QLabel *m_statusLabel;
    QLabel *m_cursorPosLabel;
    
    // 定时器用于延迟更新预览
    QTimer *m_updateTimer;
    
    QString m_currentFilePath;
    bool m_isModified;
};

#endif // MAINWINDOW_H
