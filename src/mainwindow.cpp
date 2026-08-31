#include "mainwindow.h"
#include "codeeditor.h"
#include "previewwidget.h"
#include "stylesheetloader.h"
#include "markdownparser.h"
#include "configmanager.h"
#include "findreplacedialog.h"
#include "settingsdialog.h"
#include "insertdialog.h"
#include "aboutdialog.h"

#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QScrollBar>
#include <QAction>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QLabel>
#include <QCloseEvent>
#include <QSignalMapper>
#include <QShowEvent>
#include <QPrinter>
#include <QTextCursor>
#include <QTextBlock>
#include <QRegExp>
#include <QStyle>
#include <QVBoxLayout>
#include <QDialog>
#include <QWebPage>

static const char *APP_NAME = "MarkEdit";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_editor(0)
    , m_preview(0)
    , m_splitter(0)
    , m_config(ConfigManager::instance())
    , m_styleLoader(new StyleSheetLoader(this))
    , m_previewTimer(new QTimer(this))
    , m_fileEncoding(m_config->defaultEncoding())
    , m_syncing(false)
    , m_splitSet(false)
{
    initUI();
    initMenuBar();
    initStatusBar();
    initConnections();
    loadSettings();
    setCurrentFile(QString());
    updatePreview();
    updateStatus();
}

MainWindow::~MainWindow()
{
    saveSettings();
}

// --------------------------------------------------------------------------
// UI construction
// --------------------------------------------------------------------------
void MainWindow::initUI()
{
    m_editor = new CodeEditor(this);
    m_editor->setPlainText(tr("# 欢迎使用MarkEdit\n"));
    m_editor->document()->setModified(false);
    m_preview = new PreviewWidget(this);
    m_previewTimer->setSingleShot(true);
    m_previewTimer->setInterval(500);
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(m_preview);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    setCentralWidget(m_splitter);
    setWindowTitle(APP_NAME);
}

void MainWindow::initConnections()
{
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(m_editor, SIGNAL(cursorPositionChanged()), this, SLOT(updateStatus()));
    connect(m_editor->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(syncScrollFromEditor(int)));
    connect(m_preview, SIGNAL(scrolled(int)), this, SLOT(syncScrollFromPreview(int)));
    connect(m_previewTimer, SIGNAL(timeout()), this, SLOT(updatePreview()));

    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(editorContextMenu(const QPoint &)));
    m_preview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_preview, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(previewContextMenu(const QPoint &)));
}

void MainWindow::initMenuBar()
{
    //==========================================================
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    QAction *newAct = new QAction(QIcon::fromTheme("document-new", style()->standardIcon(QStyle::SP_FileIcon)), tr("新建(&N)"), this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, SIGNAL(triggered()), this, SLOT(onNewFile()));
    fileMenu->addAction(newAct);

    QAction *openAct = new QAction(QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DirOpenIcon)), tr("打开(&O)..."), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(onOpenFile()));
    fileMenu->addAction(openAct);

    m_recentMenu = new QMenu(tr("打开最近的文件"), this);
    m_recentMenu->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    fileMenu->addMenu(m_recentMenu);
    updateRecentMenu();

    m_saveAction = new QAction(QIcon::fromTheme("document-save", style()->standardIcon(QStyle::SP_DialogSaveButton)), tr("保存(&S)"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setEnabled(false);
    connect(m_saveAction, SIGNAL(triggered()), this, SLOT(onSaveFile()));
    fileMenu->addAction(m_saveAction);

    QAction *saveAsAct = new QAction(tr("另存为(&A)..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(onSaveAs()));
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    QAction *exportHtmlAct = new QAction(tr("导出为HTML(&H)"), this);
    exportHtmlAct->setShortcut(QKeySequence(tr("Ctrl+E")));
    connect(exportHtmlAct, SIGNAL(triggered()), this, SLOT(onExportHtml()));
    fileMenu->addAction(exportHtmlAct);

    QAction *exportPdfAct = new QAction(tr("导出为PDF(&P)"), this);
    exportPdfAct->setShortcut(QKeySequence::Print);
    connect(exportPdfAct, SIGNAL(triggered()), this, SLOT(onExportPdf()));
    fileMenu->addAction(exportPdfAct);

    fileMenu->addSeparator();

    QAction *exitAct = new QAction(tr("退出(&X)"), this);
    exitAct->setShortcut(QKeySequence(tr("Alt+F4")));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(onExit()));
    fileMenu->addAction(exitAct);

    //==========================================================
    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    QAction *undoAct = new QAction(QIcon::fromTheme("edit-undo", style()->standardIcon(QStyle::SP_ArrowLeft)), tr("撤销(&U)"), this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), m_editor, SLOT(undo()));
    connect(m_editor->document(), SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)));
    editMenu->addAction(undoAct);

    QAction *redoAct = new QAction(QIcon::fromTheme("edit-redo", style()->standardIcon(QStyle::SP_ArrowRight)), tr("重做(&R)"), this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), m_editor, SLOT(redo()));
    connect(m_editor->document(), SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)));
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    QAction *cutAct = new QAction(QIcon::fromTheme("edit-cut", style()->standardIcon(QStyle::SP_DialogCancelButton)), tr("剪切(&T)"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    connect(cutAct, SIGNAL(triggered()), m_editor, SLOT(cut()));
    editMenu->addAction(cutAct);

    QAction *copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("复制(&C)"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, SIGNAL(triggered()), m_editor, SLOT(copy()));
    editMenu->addAction(copyAct);

    QAction *pasteAct = new QAction(QIcon::fromTheme("edit-paste", style()->standardIcon(QStyle::SP_DialogOkButton)), tr("粘贴(&P)"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct, SIGNAL(triggered()), m_editor, SLOT(paste()));
    editMenu->addAction(pasteAct);

    QAction *selAllAct = new QAction(tr("全选(&A)"), this);
    selAllAct->setShortcut(QKeySequence::SelectAll);
    connect(selAllAct, SIGNAL(triggered()), this, SLOT(onSelectAll()));
    editMenu->addAction(selAllAct);

    editMenu->addSeparator();

    QAction *findAct = new QAction(QIcon::fromTheme("edit-find"), tr("查找(&F)..."), this);
    findAct->setShortcut(QKeySequence::Find);
    connect(findAct, SIGNAL(triggered()), this, SLOT(onFind()));
    editMenu->addAction(findAct);

    QAction *replaceAct = new QAction(tr("替换(&R)..."), this);
    replaceAct->setShortcut(QKeySequence::Replace);
    connect(replaceAct, SIGNAL(triggered()), this, SLOT(onReplace()));
    editMenu->addAction(replaceAct);

    //==========================================================
    QMenu *viewMenu = menuBar()->addMenu(tr("查看(&V)"));

    m_lineWrapAction = new QAction(tr("自动换行"), this);
    m_lineWrapAction->setCheckable(true);
    connect(m_lineWrapAction, SIGNAL(toggled(bool)), this, SLOT(toggleLineWrap(bool)));
    viewMenu->addAction(m_lineWrapAction);

    m_lineNumberAction = new QAction(tr("显示行号"), this);
    m_lineNumberAction->setCheckable(true);
    connect(m_lineNumberAction, SIGNAL(toggled(bool)), this, SLOT(toggleLineNumber(bool)));
    viewMenu->addAction(m_lineNumberAction);

    m_whitespaceAction = new QAction(tr("显示空白"), this);
    m_whitespaceAction->setCheckable(true);
    connect(m_whitespaceAction, SIGNAL(toggled(bool)), this, SLOT(toggleWhitespace(bool)));
    viewMenu->addAction(m_whitespaceAction);

    viewMenu->addSeparator();
    m_verticalSplitAction = new QAction(tr("编辑预览"), this);
    m_verticalSplitAction->setCheckable(true);
    m_verticalSplitAction->setChecked(true);
    connect(m_verticalSplitAction, SIGNAL(triggered()), this, SLOT(setVerticalSplit()));
    viewMenu->addAction(m_verticalSplitAction);

    m_editorOnlyAction = new QAction(tr("仅编辑"), this);
    m_editorOnlyAction->setCheckable(true);
    connect(m_editorOnlyAction, SIGNAL(triggered()), this, SLOT(showEditorOnly()));
    viewMenu->addAction(m_editorOnlyAction);

    m_previewOnlyAction = new QAction(tr("仅预览"), this);
    m_previewOnlyAction->setCheckable(true);
    connect(m_previewOnlyAction, SIGNAL(triggered()), this, SLOT(showPreviewOnly()));
    viewMenu->addAction(m_previewOnlyAction);

    viewMenu->addSeparator();

    m_fullScreenAction = new QAction(tr("全屏"), this);
    m_fullScreenAction->setShortcut(QKeySequence(tr("F11")));
    m_fullScreenAction->setCheckable(true);
    connect(m_fullScreenAction, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
    viewMenu->addAction(m_fullScreenAction);

    m_statusBarAction = new QAction(tr("状态栏"), this);
    m_statusBarAction->setCheckable(true);
    m_statusBarAction->setChecked(true);
    connect(m_statusBarAction, SIGNAL(toggled(bool)), this, SLOT(toggleStatusBar(bool)));
    viewMenu->addAction(m_statusBarAction);

    //==========================================================
    QMenu *formatMenu = menuBar()->addMenu(tr("格式(&O)"));
    QAction *boldAct = new QAction(tr("粗体(&B)"), this);
    boldAct->setShortcut(QKeySequence::Bold);
    connect(boldAct, SIGNAL(triggered()), this, SLOT(formatBold()));
    formatMenu->addAction(boldAct);

    QAction *italicAct = new QAction(tr("斜体(&I)"), this);
    italicAct->setShortcut(QKeySequence::Italic);
    connect(italicAct, SIGNAL(triggered()), this, SLOT(formatItalic()));
    formatMenu->addAction(italicAct);

    QAction *underlineAct = new QAction(tr("下划线(&U)"), this);
    underlineAct->setShortcut(QKeySequence(tr("Ctrl+U")));
    connect(underlineAct, SIGNAL(triggered()), this, SLOT(formatUnderline()));
    formatMenu->addAction(underlineAct);

    QAction *strikeAct = new QAction(tr("删除线(&S)"), this);
    strikeAct->setShortcut(QKeySequence(tr("Ctrl+T")));
    connect(strikeAct, SIGNAL(triggered()), this, SLOT(formatStrikethrough()));
    formatMenu->addAction(strikeAct);

    formatMenu->addSeparator();

    QAction *h1 = new QAction(tr("标题1(&1)"), this);
    h1->setShortcut(QKeySequence(tr("Ctrl+1")));
    formatMenu->addAction(h1);

    QAction *h2 = new QAction(tr("标题2(&2)"), this);
    h2->setShortcut(QKeySequence(tr("Ctrl+2")));
    formatMenu->addAction(h2);

    QAction *h3 = new QAction(tr("标题3(&3)"), this);
    h3->setShortcut(QKeySequence(tr("Ctrl+3")));
    formatMenu->addAction(h3);

    QSignalMapper *headingMapper = new QSignalMapper(this);
    connect(h1, SIGNAL(triggered()), headingMapper, SLOT(map()));
    headingMapper->setMapping(h1, 1);
    connect(h2, SIGNAL(triggered()), headingMapper, SLOT(map()));
    headingMapper->setMapping(h2, 2);
    connect(h3, SIGNAL(triggered()), headingMapper, SLOT(map()));
    headingMapper->setMapping(h3, 3);
    connect(headingMapper, SIGNAL(mapped(int)), this, SLOT(heading(int)));

    formatMenu->addSeparator();

    QAction *ulAct = new QAction(tr("无序列表(&U)"), this);
    ulAct->setShortcut(QKeySequence(tr("Ctrl+L")));
    connect(ulAct, SIGNAL(triggered()), this, SLOT(bulletList()));
    formatMenu->addAction(ulAct);

    QAction *olAct = new QAction(tr("有序列表(&O)"), this);
    olAct->setShortcut(QKeySequence(tr("Ctrl+Shift+L")));
    connect(olAct, SIGNAL(triggered()), this, SLOT(orderedList()));
    formatMenu->addAction(olAct);

    QAction *quoteAct = new QAction(tr("引用块(&B)"), this);
    quoteAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    connect(quoteAct, SIGNAL(triggered()), this, SLOT(blockQuote()));
    formatMenu->addAction(quoteAct);

    QAction *codeAct = new QAction(tr("代码块(&C)"), this);
    codeAct->setShortcut(QKeySequence(tr("Ctrl+K")));
    connect(codeAct, SIGNAL(triggered()), this, SLOT(codeBlock()));
    formatMenu->addAction(codeAct);

    QAction *inlineCodeAct = new QAction(tr("行内代码(&I)"), this);
    inlineCodeAct->setShortcut(QKeySequence(tr("Ctrl+'")));
    connect(inlineCodeAct, SIGNAL(triggered()), this, SLOT(inlineCode()));
    formatMenu->addAction(inlineCodeAct);

    formatMenu->addSeparator();

    QAction *hrAction = new QAction(tr("插入水平线(&H)"), this);
    connect(hrAction, SIGNAL(triggered()), this, SLOT(insertHorizontalRule()));
    formatMenu->addAction(hrAction);

    QAction *linkAction = new QAction(tr("插入链接(&L)..."), this);
    connect(linkAction, SIGNAL(triggered()), this, SLOT(insertLink()));
    formatMenu->addAction(linkAction);

    QAction *imgAction = new QAction(tr("插入图片(&I)..."), this);
    connect(imgAction, SIGNAL(triggered()), this, SLOT(insertImage()));
    formatMenu->addAction(imgAction);

    //==========================================================
    QMenu *toolsMenu = menuBar()->addMenu(tr("工具(&T)"));

    m_encodingMenu = new QMenu(tr("编码(&E)"), this);
    toolsMenu->addMenu(m_encodingMenu);

    QStringList codecNames;
    codecNames << "System";
    codecNames << "UTF-8";
    codecNames << "UTF-16";
    codecNames << "UTF-32";

    QActionGroup *encodingGroup = new QActionGroup(this);
    encodingGroup->setExclusive(true);
    foreach (const QString& codec, codecNames)
    {
        QAction *act = new QAction(codec, this);
        act->setCheckable(true);
        act->setData(codec);
        act->setActionGroup(encodingGroup);
        m_encodingActions.append(act);
        m_encodingMenu->addAction(act);
    }
    connect(encodingGroup, SIGNAL(triggered(QAction*)), this, SLOT(onEncodingTriggered(QAction*)));
    updateEncodingMenu();

    toolsMenu->addSeparator();

    QAction *prefAct = new QAction(QIcon::fromTheme("preferences-system"), tr("选项(&P)..."), this);
    connect(prefAct, SIGNAL(triggered()), this, SLOT(openPreferences()));
    toolsMenu->addAction(prefAct);

    //==========================================================
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    QAction *aboutAct = new QAction(QIcon::fromTheme("help-about"), tr("关于(&A)..."), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    helpMenu->addAction(aboutAct);

    QAction *guideAct = new QAction(QIcon::fromTheme("help-contents"), tr("指南(&S)..."), this);
    guideAct->setShortcut(QKeySequence::HelpContents);
    connect(guideAct, SIGNAL(triggered()), this, SLOT(markdownGuide()));
    helpMenu->addAction(guideAct);
}

void MainWindow::initStatusBar()
{
    m_statusCursor = new QLabel(tr("行:1, 列:1"));
    m_statusCursor->setMinimumWidth(200);
    statusBar()->addPermanentWidget(m_statusCursor);

    m_statusEncoding = new QLabel("编码:"+m_fileEncoding);
    m_statusEncoding->setMinimumWidth(100);
    statusBar()->addPermanentWidget(m_statusEncoding);
}

void MainWindow::loadSettings()
{
    if (!m_config->windowGeometry().isEmpty())
        restoreGeometry(m_config->windowGeometry());
    if (!m_config->windowState().isEmpty())
        restoreState(m_config->windowState());
    applyConfigToUi();
}

void MainWindow::saveSettings()
{
    m_config->setWindowGeometry(saveGeometry());
    m_config->setWindowState(saveState());
    m_config->saveConfig();
}

void MainWindow::applyConfigToUi()
{
    m_editor->setEditorFont(m_config->editorFont());
    m_editor->setLineWrapEnabled(m_config->lineWrap());
    m_editor->setLineNumberVisible(m_config->showLineNumber());
    m_editor->setWhitespaceVisible(m_config->showWhitespace());
    m_editor->setSyntaxHighlightingEnabled(m_config->showSyntaxHighlighting());
    m_editor->setTabWidth(m_config->tabWidth());

    m_lineNumberAction->setChecked(m_config->showLineNumber());
    m_whitespaceAction->setChecked(m_config->showWhitespace());

    QString css = m_styleLoader->loadFromFile(m_config->cssFilePath());
    m_preview->setCSS(css);
}

// --------------------------------------------------------------------------
// File operations
// --------------------------------------------------------------------------
void MainWindow::onNewFile()
{
    if (!maybeSave())
        return;
    m_editor->clear();
    m_editor->document()->setModified(false);
    setCurrentFile(QString());
    updateEncodingMenu();
    updatePreview();
}

void MainWindow::onOpenFile()
{
    if (!maybeSave())
        return;
    QString path = QFileDialog::getOpenFileName(this, tr("打开文件"), QString(), tr("Markdown(*.md);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    loadFile(path);
}

void MainWindow::onOpenRecent()
{
    QAction *a = qobject_cast<QAction *>(sender());
    if (!a)
        return;
    QString path = a->data().toString();
    if (!QFile::exists(path))
    {
        QMessageBox::warning(this, tr("打开最近的文件"), tr("文件已不存在:\n%1").arg(path));
        m_config->addRecentFile(path);
        m_config->clearRecentFiles();
        QStringList cleaned;
        for (const QString &f : m_config->recentFiles())
            if (f != path) cleaned.append(f);
        for (const QString &f : cleaned)
            m_config->addRecentFile(f);
        updateRecentMenu();
        return;
    }
    if (!maybeSave())
        return;
    loadFile(path);
}

bool MainWindow::loadFile(const QString &path)
{
    return loadFileWithEncoding(path, m_config->defaultEncoding());
}

bool MainWindow::loadFileWithEncoding(const QString &path, const QString &encoding)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("打开"), tr("无法打开文件:\n%1").arg(path));
        return false;
    }
    QByteArray raw = file.readAll();
    file.close();

    QTextCodec *codec = QTextCodec::codecForName(encoding.toAscii());
    QString txt = codec->toUnicode(raw);
    m_editor->setPlainText(txt);
    m_editor->document()->setModified(false);
    m_fileEncoding = QString::fromLatin1(codec->name());
    m_statusEncoding->setText("编码:"+m_fileEncoding);
    setCurrentFile(path);
    updateEncodingMenu();
    updatePreview();
    return true;
}

void MainWindow::onEncodingSelected(const QString &codecName)
{
    if (m_currentFile.isEmpty())
        return;

    if (m_editor->document()->isModified() && !maybeSave())
        return;

    if (!loadFileWithEncoding(m_currentFile, codecName))
        return;

    m_config->setDefaultEncoding(codecName);
    m_config->saveConfig();
}

void MainWindow::updateEncodingMenu()
{
    if (!m_encodingMenu)
        return;

    m_encodingMenu->setEnabled(!m_currentFile.isEmpty());
    foreach (QAction *act, m_encodingActions)
    {
        act->setChecked(act->data().toString() == m_fileEncoding);
    }
}

void MainWindow::onSaveFile()
{
    if (m_currentFile.isEmpty())
        onSaveAs();
    else
    {
        saveFile(m_currentFile);
        m_editor->document()->setModified(false);
    }
}

void MainWindow::onSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("保存 Markdown 文件"),
                   m_currentFile.isEmpty() ? "untitled.md" : m_currentFile,
                   tr("Markdown 文件 (*.md);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    if (saveFile(path))
    {
        setCurrentFile(path);
        m_editor->document()->setModified(false);
    }
}

bool MainWindow::saveFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("保存"), tr("无法保存文件:\n%1").arg(path));
        return false;
    }
    QTextStream out(&file);
    QTextCodec *codec = QTextCodec::codecForName(m_fileEncoding.toLatin1());
    if (!codec)
        codec = QTextCodec::codecForName("UTF-8");
    out.setCodec(codec);
    out << m_editor->toPlainText();
    file.close();
    statusBar()->showMessage(tr("已保存 %1").arg(path), 3000);
    return true;
}

void MainWindow::onExportHtml()
{
    QString path = QFileDialog::getSaveFileName(this, tr("导出 HTML"),
                   m_currentFile.isEmpty() ? "output.html" : QFileInfo(m_currentFile).baseName() + ".html",
                   tr("HTML 文件 (*.html *.htm);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    MarkdownParser parser;
    QString body = parser.parse(m_editor->toPlainText());
    QString css = m_styleLoader->loadFromFile(m_config->cssFilePath());
    QString html = QString(
                       "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                       "<title>%1</title><style>%2</style></head><body>%3</body></html>")
                   .arg(QFileInfo(m_currentFile).baseName())
                   .arg(css).arg(body);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("导出"), tr("无法写入文件:\n%1").arg(path));
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << html;
    file.close();
    statusBar()->showMessage(tr("已导出 HTML: %1").arg(path), 3000);
}

void MainWindow::onExportPdf()
{
    QString path = QFileDialog::getSaveFileName(this, tr("导出 PDF"),
                   m_currentFile.isEmpty() ? "output.pdf" : QFileInfo(m_currentFile).baseName() + ".pdf",
                   tr("PDF 文件 (*.pdf);;所有文件 (*)"));
    if (path.isEmpty())
        return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    m_preview->print(&printer);
    statusBar()->showMessage(tr("已导出 PDF: %1").arg(path), 3000);
}

// --------------------------------------------------------------------------
// Edit operations
// --------------------------------------------------------------------------
void MainWindow::onSelectAll()
{
    m_editor->selectAll();
}

void MainWindow::onFind()
{
    static FindReplaceDialog *dlg = 0;
    if (!dlg)
        dlg = new FindReplaceDialog(m_editor, this);
    if (!dlg->isVisible())
    {
        QTextCursor cur = m_editor->textCursor();
        if (cur.hasSelection())
            dlg->setFindText(cur.selectedText());
        dlg->show();
        dlg->raise();
        dlg->activateWindow();
    }
}

void MainWindow::onReplace()
{
    static FindReplaceDialog *dlg = 0;
    if (!dlg)
        dlg = new FindReplaceDialog(m_editor, this);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

// --------------------------------------------------------------------------
// View operations
// --------------------------------------------------------------------------
void MainWindow::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::toggleLineWrap(bool checked)
{
    m_config->setLineWrap(checked);
    m_editor->setLineWrapEnabled(checked);
    m_config->saveConfig();
}

void MainWindow::toggleLineNumber(bool checked)
{
    m_config->setShowLineNumber(checked);
    m_editor->setLineNumberVisible(checked);
    m_config->saveConfig();
}

void MainWindow::toggleWhitespace(bool checked)
{
    m_config->setShowWhitespace(checked);
    m_editor->setWhitespaceVisible(checked);
    m_config->saveConfig();
}

void MainWindow::setVerticalSplit()
{
    m_editor->setVisible(true);
    m_preview->setVisible(true);
    m_splitter->setOrientation(Qt::Horizontal);
    m_verticalSplitAction->setChecked(true);
    m_editorOnlyAction->setChecked(false);
    m_previewOnlyAction->setChecked(false);
}

void MainWindow::showEditorOnly()
{
    m_editor->setVisible(true);
    m_preview->setVisible(false);
    m_verticalSplitAction->setChecked(false);
    m_editorOnlyAction->setChecked(true);
    m_previewOnlyAction->setChecked(false);
}

void MainWindow::showPreviewOnly()
{
    m_editor->setVisible(false);
    m_preview->setVisible(true);
    m_verticalSplitAction->setChecked(false);
    m_editorOnlyAction->setChecked(false);
    m_previewOnlyAction->setChecked(true);
}

void MainWindow::toggleStatusBar(bool checked)
{
    statusBar()->setVisible(checked);
}

// --------------------------------------------------------------------------
// Format operations
// --------------------------------------------------------------------------
void MainWindow::formatBold()
{
    wrapSelection("**", "**");
}
void MainWindow::formatItalic()
{
    wrapSelection("*", "*");
}
void MainWindow::formatUnderline()
{
    wrapSelection("<u>", "</u>");
}
void MainWindow::formatStrikethrough()
{
    wrapSelection("~~", "~~");
}
void MainWindow::inlineCode()
{
    wrapSelection("`", "`");
}

void MainWindow::heading(int level)
{
    QString prefix = QString("#").repeated(level) + " ";
    QTextCursor cur = m_editor->textCursor();
    if (!cur.hasSelection())
        cur.select(QTextCursor::LineUnderCursor);

    int start = cur.selectionStart();
    int end = cur.selectionEnd();
    QTextBlock block = m_editor->document()->findBlock(start);
    QTextBlock endBlock = m_editor->document()->findBlock(end);
    QString newText;
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next())
    {
        QString line = block.text();
        QString stripped = line;
        QRegExp headerRe("^#{1,6}\\s+");
        if (headerRe.indexIn(stripped) == 0)
            stripped.remove(0, headerRe.matchedLength());
        if (line.startsWith(prefix))
            newText += stripped + "\n";
        else
            newText += prefix + stripped + "\n";
    }
    if (newText.endsWith("\n"))
        newText.chop(1);

    QTextCursor sel(m_editor->document());
    sel.setPosition(start);
    sel.setPosition(end, QTextCursor::KeepAnchor);
    sel.insertText(newText);
    m_editor->setTextCursor(cur);
}

void MainWindow::bulletList()
{
    applyLinePrefix("- ");
}

void MainWindow::orderedList()
{
    QTextCursor cur = m_editor->textCursor();
    if (!cur.hasSelection())
        cur.select(QTextCursor::LineUnderCursor);
    int start = cur.selectionStart();
    int end = cur.selectionEnd();
    QTextBlock block = m_editor->document()->findBlock(start);
    QTextBlock endBlock = m_editor->document()->findBlock(end);
    QString newText;
    int n = 1;
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next())
    {
        QString line = block.text();
        QString stripped = line;
        QRegExp listRe("^\\s*\\d+\\.\\s+");
        if (listRe.indexIn(stripped) == 0)
            stripped.remove(0, listRe.matchedLength());
        newText += QString("%1. %2\n").arg(n++).arg(stripped);
    }
    if (newText.endsWith("\n"))
        newText.chop(1);
    QTextCursor sel(m_editor->document());
    sel.setPosition(start);
    sel.setPosition(end, QTextCursor::KeepAnchor);
    sel.insertText(newText);
    m_editor->setTextCursor(cur);
}

void MainWindow::blockQuote()
{
    applyLinePrefix("> ");
}

void MainWindow::codeBlock()
{
    QTextCursor cur = m_editor->textCursor();
    if (cur.hasSelection())
    {
        QString sel = cur.selectedText();
        cur.insertText("\n```\n" + sel + "\n```\n");
    }
    else
    {
        int pos = cur.position();
        cur.insertText("\n```\n\n```\n");
        cur.setPosition(pos + 5); // inside the fence
        m_editor->setTextCursor(cur);
    }
}

void MainWindow::insertLink()
{
    InsertDialog dlg("link", this);
    QTextCursor cur = m_editor->textCursor();
    if (cur.hasSelection())
        dlg.setText(cur.selectedText());
    if (dlg.exec() == QDialog::Accepted)
    {
        QString text = dlg.text().isEmpty() && cur.hasSelection() ? cur.selectedText() : dlg.text();
        QString url = dlg.url();
        QString md = QString("[%1](%2)").arg(text).arg(url);
        if (cur.hasSelection())
            cur.insertText(md);
        else
            insertAtCursor(md);
    }
}

void MainWindow::insertImage()
{
    InsertDialog dlg("image", this);
    QTextCursor cur = m_editor->textCursor();
    if (cur.hasSelection())
        dlg.setText(cur.selectedText());
    if (dlg.exec() == QDialog::Accepted)
    {
        QString text = dlg.text().isEmpty() ? dlg.url() : dlg.text();
        QString url = dlg.url();
        QString md = QString("![%1](%2)").arg(text).arg(url);
        if (cur.hasSelection())
            cur.insertText(md);
        else
            insertAtCursor(md);
    }
}

void MainWindow::insertHorizontalRule()
{
    insertAtCursor("\n---\n");
}

void MainWindow::openPreferences()
{
    SettingsDialog dlg(m_config, this);
    dlg.exec();
}

void MainWindow::onEncodingTriggered(QAction* action)
{
    if (action)
    {
        QString codec = action->data().toString();
        onEncodingSelected(codec);
    }
}

// --------------------------------------------------------------------------
// Help operations
// --------------------------------------------------------------------------
void MainWindow::about()
{
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::markdownGuide()
{
    const QString guide =
        "<h1>Markdown语法指南</h1>"
        "<h2>标题</h2><br><pre><code># H1\n## H2\n### H3</code></pre>"
        "<h2>强调</h2><br><pre><code>**粗体**  *斜体*  ~~删除线~~  `代码`</code></pre>"
        "<h2>列表</h2><br><pre><code>- 项目\n- 项目\n\n1. 第一\n2. 第二</code></pre>"
        "<h2>引用块</h2><br><pre><code>&gt; 引用文本</code></pre>"
        "<h2>代码块</h2><br><pre><code>```\n代码内容\n```</code></pre>"
        "<h2>链接和图片</h2><br><pre><code>[文本](https://example.com)\n"
        "![说明文字](图片.png)</code></pre>"
        "<h2>水平线</h2><pre><code>---</code></pre>";
    PreviewWidget *w = new PreviewWidget(this);
    //w->setCSS(StyleSheetLoader::getDefaultCSS());
    w->setMarkdown(guide);
    QDialog dlg(this);
    dlg.setWindowTitle(tr("语法指南"));
    dlg.resize(640, 520);
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    lay->addWidget(w);
    dlg.exec();
}

// --------------------------------------------------------------------------
// Preview / editor plumbing
// --------------------------------------------------------------------------
void MainWindow::onTextChanged()
{
    m_saveAction->setEnabled(m_editor->document()->isModified());
    if (m_config->autoRefresh())
        m_previewTimer->start();
}

void MainWindow::updatePreview()
{
    m_preview->setMarkdown(m_editor->toPlainText());
}

void MainWindow::updateStatus()
{
    QTextCursor cur = m_editor->textCursor();
    int line = cur.blockNumber() + 1;
    int col = cur.positionInBlock() + 1;
    m_statusCursor->setText(tr("行:%1, 列:%2").arg(line).arg(col));
}

void MainWindow::syncScrollFromEditor(int value)
{
    if (m_syncing || !m_config->syncScroll() || !m_preview->isVisible())
        return;
    int max = m_editor->verticalScrollBar()->maximum();
    float ratio = max > 0 ? (float)value / max : 0.0f;
    m_syncing = true;
    m_preview->setScrollRatio(ratio);
    m_syncing = false;
}

void MainWindow::syncScrollFromPreview(int value)
{
    if (m_syncing || !m_config->syncScroll() || !m_editor->isVisible())
        return;
    int previewMax = m_preview->scrollMaximum();
    float ratio = previewMax > 0 ? (float)value / previewMax : 0.0f;
    int editorMax = m_editor->verticalScrollBar()->maximum();
    m_syncing = true;
    m_editor->verticalScrollBar()->setValue(static_cast<int>(editorMax * ratio));
    m_syncing = false;
}

// --------------------------------------------------------------------------
// Context menus
// --------------------------------------------------------------------------
void MainWindow::editorContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *undo = menu.addAction(tr("撤销"));
    connect(undo, SIGNAL(triggered()), m_editor, SLOT(undo()));
    QAction *redo = menu.addAction(tr("重做"));
    connect(redo, SIGNAL(triggered()), m_editor, SLOT(redo()));
    menu.addSeparator();
    QAction *cut = menu.addAction(tr("剪切"));
    connect(cut, SIGNAL(triggered()), m_editor, SLOT(cut()));
    QAction *copy = menu.addAction(tr("复制"));
    connect(copy, SIGNAL(triggered()), m_editor, SLOT(copy()));
    QAction *paste = menu.addAction(tr("粘贴"));
    connect(paste, SIGNAL(triggered()), m_editor, SLOT(paste()));
    menu.addSeparator();
    QAction *selAll = menu.addAction(tr("全选"));
    connect(selAll, SIGNAL(triggered()), this, SLOT(onSelectAll()));
    menu.addSeparator();
    QAction *find = menu.addAction(tr("查找..."));
    connect(find, SIGNAL(triggered()), this, SLOT(onFind()));
    Q_UNUSED(pos);
    menu.exec(m_editor->viewport()->mapToGlobal(pos));
}

void MainWindow::previewContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *copy = menu.addAction(tr("复制"));
    connect(copy, SIGNAL(triggered()), this, SLOT(previewCopy()));
    QAction *selAll = menu.addAction(tr("全选"));
    connect(selAll, SIGNAL(triggered()), this, SLOT(previewSelectAll()));
    menu.exec(m_preview->mapToGlobal(pos));
}

void MainWindow::previewCopy()
{
    m_preview->triggerPageAction(QWebPage::Copy);
}

void MainWindow::previewSelectAll()
{
    m_preview->triggerPageAction(QWebPage::SelectAll);
}

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------
void MainWindow::wrapSelection(const QString &before, const QString &after)
{
    QTextCursor cur = m_editor->textCursor();
    if (cur.hasSelection())
    {
        QString sel = cur.selectedText();
        int selStart = cur.selectionStart();
        if (sel.startsWith(before) && sel.endsWith(after) &&
                sel.length() >= before.length() + after.length())
        {
            QString inner = sel.mid(before.length(),
                                    sel.length() - before.length() - after.length());
            cur.insertText(inner);
            cur.setPosition(selStart);
            cur.setPosition(selStart + inner.length(), QTextCursor::KeepAnchor);
        }
        else
        {
            cur.insertText(before + sel + after);
            cur.setPosition(selStart);
            cur.setPosition(selStart + before.length() + sel.length(), QTextCursor::KeepAnchor);
        }
    }
    else
    {
        int pos = cur.position();
        cur.insertText(before + after);
        cur.setPosition(pos + before.length());
        m_editor->setTextCursor(cur);
        return;
    }
    m_editor->setTextCursor(cur);
}

void MainWindow::applyLinePrefix(const QString &prefix, bool toggle)
{
    QTextCursor cur = m_editor->textCursor();
    if (!cur.hasSelection())
        cur.select(QTextCursor::LineUnderCursor);
    int start = cur.selectionStart();
    int end = cur.selectionEnd();
    QTextBlock block = m_editor->document()->findBlock(start);
    QTextBlock endBlock = m_editor->document()->findBlock(end);
    QString newText;
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next())
    {
        QString line = block.text();
        if (toggle && line.startsWith(prefix))
            newText += line.mid(prefix.length()) + "\n";
        else if (toggle)
            newText += prefix + line + "\n";
        else
            newText += prefix + line + "\n";
    }
    if (newText.endsWith("\n"))
        newText.chop(1);
    QTextCursor sel(m_editor->document());
    sel.setPosition(start);
    sel.setPosition(end, QTextCursor::KeepAnchor);
    sel.insertText(newText);
    m_editor->setTextCursor(cur);
}

void MainWindow::insertAtCursor(const QString &text)
{
    QTextCursor cur = m_editor->textCursor();
    cur.insertText(text);
    m_editor->setTextCursor(cur);
}

bool MainWindow::maybeSave()
{
    if (!m_editor->document()->isModified())
        return true;
    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("MarkEdit"),
                                      tr("文档已被修改。\n是否保存更改？"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
    {
        onSaveFile();
        if (m_editor->document()->isModified())
            return false;   // save was cancelled or failed
    }
    else if (ret == QMessageBox::Cancel)
    {
        return false;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    QString title = APP_NAME;
    if (path.isEmpty())
        title = tr("未命名") + " - " + title;
    else
    {
        title = QFileInfo(path).fileName() + " - " + title;
        m_config->addRecentFile(path);
        updateRecentMenu();
    }
    setWindowTitle(title);
    m_saveAction->setEnabled(m_editor->document()->isModified());
    if (!path.isEmpty())
        m_config->saveConfig();
}

void MainWindow::updateRecentMenu()
{
    m_recentMenu->clear();
    QStringList recent = m_config->recentFiles(10);
    if (recent.isEmpty())
    {
        QAction *none = new QAction(tr("(无最近文件)"), this);
        none->setEnabled(false);
        m_recentMenu->addAction(none);
        return;
    }
    for (const QString &f : recent)
    {
        QAction *a = new QAction(QFileInfo(f).fileName(), this);
        a->setData(f);
        a->setToolTip(f);
        connect(a, SIGNAL(triggered()), this, SLOT(onOpenRecent()));
        m_recentMenu->addAction(a);
    }
    m_recentMenu->addSeparator();
    QAction *clear = new QAction(tr("清空列表"), this);
    connect(clear, SIGNAL(triggered()), this, SLOT(clearRecentFilesAction()));
    m_recentMenu->addAction(clear);
}

void MainWindow::clearRecentFilesAction()
{
    m_config->clearRecentFiles();
    m_config->saveConfig();
    updateRecentMenu();
}

// --------------------------------------------------------------------------
// Close event
// --------------------------------------------------------------------------
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_splitSet)
    {
        m_splitSet = true;
        int half = m_splitter->width() / 2;
        m_splitter->setSizes(QList<int>() << half << half);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::onExit()
{
    close();
}
