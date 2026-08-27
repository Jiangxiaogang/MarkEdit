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

#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QScrollBar>
#include <QLabel>
#include <QCloseEvent>
#include <QShowEvent>
#include <QPrinter>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextDocument>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QClipboard>
#include <QApplication>
#include <QStyle>
#include <QDateTime>
#include <QVBoxLayout>
#include <QDialog>
#include <QWebPage>

static const char *APP_NAME = "MarkEdit";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_editor(nullptr)
    , m_preview(nullptr)
    , m_splitter(nullptr)
    , m_config(ConfigManager::instance())
    , m_styleLoader(new StyleSheetLoader(this))
    , m_previewTimer(new QTimer(this))
    , m_syncing(false)
    , m_splitSet(false)
{
    initUI();
    initMenus();
    initToolbar();
    initStatusBar();
    initConnections();
    loadSettings();

    m_previewTimer->setSingleShot(true);
    m_previewTimer->setInterval(300);

    // Initial content
    m_editor->setPlainText(tr("# Welcome to MarkEdit\n\n"
                              "A cross-platform **Markdown** editor built with Qt.\n\n"
                              "- Type on the left\n"
                              "- Preview updates on the right\n\n"
                              "> Select *Format* from the menu to add Markdown syntax."));
    m_editor->document()->setModified(false);
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
    m_preview = new PreviewWidget(this);

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
    connect(m_editor, &CodeEditor::textChanged, this, &MainWindow::onTextChanged);
    connect(m_editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateStatus);
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MainWindow::syncScrollFromEditor);
    connect(m_preview, &PreviewWidget::scrolled, this, &MainWindow::syncScrollFromPreview);

    connect(m_preview, &PreviewWidget::cssFailed, this, [this](const QString &e) {
        statusBar()->showMessage(e, 4000);
    });

    connect(m_config, &ConfigManager::configurationChanged, this, &MainWindow::applyConfigToUi);

    connect(m_previewTimer, &QTimer::timeout, this, &MainWindow::updatePreview);

    m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_editor, &QWidget::customContextMenuRequested,
            this, &MainWindow::editorContextMenu);
    m_preview->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_preview, &QWidget::customContextMenuRequested,
            this, &MainWindow::previewContextMenu);
}

// --------------------------------------------------------------------------
// Menus
// --------------------------------------------------------------------------
void MainWindow::initMenus()
{
    // ---- File ----
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *newAct = new QAction(QIcon::fromTheme("document-new",
        style()->standardIcon(QStyle::SP_FileIcon)), tr("&New"), this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::onNewFile);
    fileMenu->addAction(newAct);

    QAction *openAct = new QAction(QIcon::fromTheme("document-open",
        style()->standardIcon(QStyle::SP_DirOpenIcon)), tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addAction(openAct);

    m_recentMenu = new QMenu(tr("Open Recent"), this);
    m_recentMenu->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    fileMenu->addMenu(m_recentMenu);
    updateRecentMenu();

    m_saveAction = new QAction(QIcon::fromTheme("document-save",
        style()->standardIcon(QStyle::SP_DialogSaveButton)), tr("&Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setEnabled(false);
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    fileMenu->addAction(m_saveAction);

    QAction *saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::onSaveAs);
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();

    QAction *exportHtmlAct = new QAction(tr("Export as &HTML"), this);
    exportHtmlAct->setShortcut(QKeySequence(tr("Ctrl+E")));
    connect(exportHtmlAct, &QAction::triggered, this, &MainWindow::onExportHtml);
    fileMenu->addAction(exportHtmlAct);

    QAction *exportPdfAct = new QAction(tr("Export as &PDF"), this);
    exportPdfAct->setShortcut(QKeySequence::Print);
    connect(exportPdfAct, &QAction::triggered, this, &MainWindow::onExportPdf);
    fileMenu->addAction(exportPdfAct);

    fileMenu->addSeparator();

    QAction *exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(QKeySequence(tr("Alt+F4")));
    connect(exitAct, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAct);

    // ---- Edit ----
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    QAction *undoAct = new QAction(QIcon::fromTheme("edit-undo",
        style()->standardIcon(QStyle::SP_ArrowLeft)), tr("&Undo"), this);
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setEnabled(false);
    connect(undoAct, &QAction::triggered, m_editor, &CodeEditor::undo);
    connect(m_editor->document(), &QTextDocument::undoAvailable, undoAct, &QAction::setEnabled);
    editMenu->addAction(undoAct);

    QAction *redoAct = new QAction(QIcon::fromTheme("edit-redo",
        style()->standardIcon(QStyle::SP_ArrowRight)), tr("&Redo"), this);
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setEnabled(false);
    connect(redoAct, &QAction::triggered, m_editor, &CodeEditor::redo);
    connect(m_editor->document(), &QTextDocument::redoAvailable, redoAct, &QAction::setEnabled);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    QAction *cutAct = new QAction(QIcon::fromTheme("edit-cut",
        style()->standardIcon(QStyle::SP_DialogCancelButton)), tr("Cu&t"), this);
    cutAct->setShortcut(QKeySequence::Cut);
    connect(cutAct, &QAction::triggered, m_editor, &CodeEditor::cut);
    editMenu->addAction(cutAct);

    QAction *copyAct = new QAction(QIcon::fromTheme("edit-copy"), tr("&Copy"), this);
    copyAct->setShortcut(QKeySequence::Copy);
    connect(copyAct, &QAction::triggered, m_editor, &CodeEditor::copy);
    editMenu->addAction(copyAct);

    QAction *pasteAct = new QAction(QIcon::fromTheme("edit-paste",
        style()->standardIcon(QStyle::SP_DialogOkButton)), tr("&Paste"), this);
    pasteAct->setShortcut(QKeySequence::Paste);
    connect(pasteAct, &QAction::triggered, m_editor, &CodeEditor::paste);
    editMenu->addAction(pasteAct);

    QAction *selAllAct = new QAction(tr("Select &All"), this);
    selAllAct->setShortcut(QKeySequence::SelectAll);
    connect(selAllAct, &QAction::triggered, this, &MainWindow::onSelectAll);
    editMenu->addAction(selAllAct);

    editMenu->addSeparator();

    QAction *findAct = new QAction(QIcon::fromTheme("edit-find"), tr("&Find..."), this);
    findAct->setShortcut(QKeySequence::Find);
    connect(findAct, &QAction::triggered, this, &MainWindow::onFind);
    editMenu->addAction(findAct);

    QAction *replaceAct = new QAction(tr("&Replace..."), this);
    replaceAct->setShortcut(QKeySequence::Replace);
    connect(replaceAct, &QAction::triggered, this, &MainWindow::onReplace);
    editMenu->addAction(replaceAct);

    // ---- View ----
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    m_fullScreenAction = new QAction(tr("&Full Screen"), this);
    m_fullScreenAction->setShortcut(QKeySequence(tr("F11")));
    m_fullScreenAction->setCheckable(true);
    connect(m_fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    viewMenu->addAction(m_fullScreenAction);

    viewMenu->addSeparator();

    m_lineNumbersAction = new QAction(tr("Show &Line Numbers"), this);
    m_lineNumbersAction->setCheckable(true);
    connect(m_lineNumbersAction, &QAction::toggled, this, &MainWindow::toggleLineNumbers);
    viewMenu->addAction(m_lineNumbersAction);

    m_whitespaceAction = new QAction(tr("Show &Whitespace"), this);
    m_whitespaceAction->setShortcut(QKeySequence(tr("Ctrl+Shift+B")));
    m_whitespaceAction->setCheckable(true);
    connect(m_whitespaceAction, &QAction::toggled, this, &MainWindow::toggleWhitespace);
    viewMenu->addAction(m_whitespaceAction);

    viewMenu->addSeparator();

    m_verticalSplitAction = new QAction(tr("&Vertical Split"), this);
    m_verticalSplitAction->setCheckable(true);
    m_verticalSplitAction->setChecked(true);
    connect(m_verticalSplitAction, &QAction::triggered, this, &MainWindow::setVerticalSplit);
    viewMenu->addAction(m_verticalSplitAction);

    m_editorOnlyAction = new QAction(tr("&Editor Only"), this);
    m_editorOnlyAction->setCheckable(true);
    connect(m_editorOnlyAction, &QAction::triggered, this, &MainWindow::showEditorOnly);
    viewMenu->addAction(m_editorOnlyAction);

    m_previewOnlyAction = new QAction(tr("&Preview Only"), this);
    m_previewOnlyAction->setCheckable(true);
    connect(m_previewOnlyAction, &QAction::triggered, this, &MainWindow::showPreviewOnly);
    viewMenu->addAction(m_previewOnlyAction);

    viewMenu->addSeparator();

    m_statusBarAction = new QAction(tr("&Status Bar"), this);
    m_statusBarAction->setCheckable(true);
    m_statusBarAction->setChecked(true);
    connect(m_statusBarAction, &QAction::toggled, this, &MainWindow::toggleStatusBar);
    viewMenu->addAction(m_statusBarAction);

    // ---- Format ----
    QMenu *formatMenu = menuBar()->addMenu(tr("F&ormat"));
    QAction *boldAct = new QAction(tr("&Bold"), this);
    boldAct->setShortcut(QKeySequence::Bold);
    connect(boldAct, &QAction::triggered, this, &MainWindow::formatBold);
    formatMenu->addAction(boldAct);

    QAction *italicAct = new QAction(tr("&Italic"), this);
    italicAct->setShortcut(QKeySequence::Italic);
    connect(italicAct, &QAction::triggered, this, &MainWindow::formatItalic);
    formatMenu->addAction(italicAct);

    QAction *underlineAct = new QAction(tr("&Underline"), this);
    underlineAct->setShortcut(QKeySequence(tr("Ctrl+U")));
    connect(underlineAct, &QAction::triggered, this, &MainWindow::formatUnderline);
    formatMenu->addAction(underlineAct);

    QAction *strikeAct = new QAction(tr("&Strikethrough"), this);
    strikeAct->setShortcut(QKeySequence(tr("Ctrl+T")));
    connect(strikeAct, &QAction::triggered, this, &MainWindow::formatStrikethrough);
    formatMenu->addAction(strikeAct);

    formatMenu->addSeparator();

    QAction *h1 = new QAction(tr("Heading &1"), this);
    h1->setShortcut(QKeySequence(tr("Ctrl+1")));
    connect(h1, &QAction::triggered, this, [this]() { heading(1); });
    formatMenu->addAction(h1);

    QAction *h2 = new QAction(tr("Heading &2"), this);
    h2->setShortcut(QKeySequence(tr("Ctrl+2")));
    connect(h2, &QAction::triggered, this, [this]() { heading(2); });
    formatMenu->addAction(h2);

    QAction *h3 = new QAction(tr("Heading &3"), this);
    h3->setShortcut(QKeySequence(tr("Ctrl+3")));
    connect(h3, &QAction::triggered, this, [this]() { heading(3); });
    formatMenu->addAction(h3);

    formatMenu->addSeparator();

    QAction *ulAct = new QAction(tr("&Unordered List"), this);
    ulAct->setShortcut(QKeySequence(tr("Ctrl+L")));
    connect(ulAct, &QAction::triggered, this, &MainWindow::bulletList);
    formatMenu->addAction(ulAct);

    QAction *olAct = new QAction(tr("&Ordered List"), this);
    olAct->setShortcut(QKeySequence(tr("Ctrl+Shift+L")));
    connect(olAct, &QAction::triggered, this, &MainWindow::orderedList);
    formatMenu->addAction(olAct);

    QAction *quoteAct = new QAction(tr("&Block Quote"), this);
    quoteAct->setShortcut(QKeySequence(tr("Ctrl+Q")));
    connect(quoteAct, &QAction::triggered, this, &MainWindow::blockQuote);
    formatMenu->addAction(quoteAct);

    QAction *codeAct = new QAction(tr("&Code Block"), this);
    codeAct->setShortcut(QKeySequence(tr("Ctrl+K")));
    connect(codeAct, &QAction::triggered, this, &MainWindow::codeBlock);
    formatMenu->addAction(codeAct);

    QAction *inlineCodeAct = new QAction(tr("&Inline Code"), this);
    inlineCodeAct->setShortcut(QKeySequence(tr("Ctrl+'")));
    connect(inlineCodeAct, &QAction::triggered, this, &MainWindow::inlineCode);
    formatMenu->addAction(inlineCodeAct);

    formatMenu->addSeparator();

    QAction *linkAction = new QAction(tr("Insert &Link..."), this);
    connect(linkAction, &QAction::triggered, this, &MainWindow::insertLink);
    formatMenu->addAction(linkAction);

    QAction *imgAction = new QAction(tr("Insert &Image..."), this);
    connect(imgAction, &QAction::triggered, this, &MainWindow::insertImage);
    formatMenu->addAction(imgAction);

    QAction *hrAction = new QAction(tr("Insert &Horizontal Rule"), this);
    connect(hrAction, &QAction::triggered, this, &MainWindow::insertHorizontalRule);
    formatMenu->addAction(hrAction);

    // ---- Tools ----
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    QAction *cssAct = new QAction(tr("Select CSS &Style..."), this);
    connect(cssAct, &QAction::triggered, this, &MainWindow::selectCss);
    toolsMenu->addAction(cssAct);

    QAction *resetCssAct = new QAction(tr("&Reset CSS Style"), this);
    connect(resetCssAct, &QAction::triggered, this, &MainWindow::resetCss);
    toolsMenu->addAction(resetCssAct);

    toolsMenu->addSeparator();

    QAction *prefAct = new QAction(QIcon::fromTheme("preferences-system"),
        tr("&Preferences..."), this);
    prefAct->setShortcut(QKeySequence(tr("Ctrl+,")));
    connect(prefAct, &QAction::triggered, this, &MainWindow::openPreferences);
    toolsMenu->addAction(prefAct);

    // ---- Help ----
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);
    helpMenu->addAction(aboutAct);

    helpMenu->addSeparator();

    QAction *guideAct = new QAction(tr("Markdown &Syntax Guide"), this);
    guideAct->setShortcut(QKeySequence::HelpContents);
    connect(guideAct, &QAction::triggered, this, &MainWindow::markdownGuide);
    helpMenu->addAction(guideAct);
}

void MainWindow::initToolbar()
{
    QToolBar *tb = new QToolBar(tr("Main Toolbar"), this);
    tb->setObjectName("mainToolBar");
    addToolBar(tb);

    QAction *newAct = new QAction(style()->standardIcon(QStyle::SP_FileIcon), tr("New"), this);
    newAct->setShortcut(QKeySequence::New);
    connect(newAct, &QAction::triggered, this, &MainWindow::onNewFile);
    tb->addAction(newAct);

    QAction *openAct = new QAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("Open"), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);
    tb->addAction(openAct);

    tb->addAction(m_saveAction);
    tb->addSeparator();

    QAction *undoAct = new QAction(style()->standardIcon(QStyle::SP_ArrowLeft), tr("Undo"), this);
    connect(undoAct, &QAction::triggered, m_editor, &CodeEditor::undo);
    tb->addAction(undoAct);
    QAction *redoAct = new QAction(style()->standardIcon(QStyle::SP_ArrowRight), tr("Redo"), this);
    connect(redoAct, &QAction::triggered, m_editor, &CodeEditor::redo);
    tb->addAction(redoAct);
    tb->addSeparator();

    QAction *boldAct = new QAction(tr("B"), this);
    boldAct->setToolTip(tr("Bold"));
    connect(boldAct, &QAction::triggered, this, &MainWindow::formatBold);
    tb->addAction(boldAct);
    QAction *italicAct = new QAction(tr("I"), this);
    italicAct->setToolTip(tr("Italic"));
    connect(italicAct, &QAction::triggered, this, &MainWindow::formatItalic);
    tb->addAction(italicAct);
    tb->addSeparator();

    QAction *refreshAct = new QAction(tr("Refresh Preview"), this);
    connect(refreshAct, &QAction::triggered, this, &MainWindow::updatePreview);
    tb->addAction(refreshAct);
}

void MainWindow::initStatusBar()
{
    m_statusCursor = new QLabel(tr("Line 1, Col 1"));
    m_statusCursor->setMinimumWidth(120);
    statusBar()->addPermanentWidget(m_statusCursor);

    m_statusWords = new QLabel(tr("Words: 0"));
    m_statusWords->setMinimumWidth(100);
    statusBar()->addPermanentWidget(m_statusWords);

    statusBar()->showMessage(tr("Ready"));
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
    m_editor->setLineNumbersVisible(m_config->showLineNumbers());
    m_editor->setWhitespaceVisible(m_config->showWhitespace());
    m_editor->setSyntaxHighlightingEnabled(m_config->showSyntaxHighlighting());
    m_editor->setTabWidth(m_config->tabWidth());

    m_lineNumbersAction->setChecked(m_config->showLineNumbers());
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
    updatePreview();
}

void MainWindow::onOpenFile()
{
    if (!maybeSave())
        return;
    QString path = QFileDialog::getOpenFileName(this, tr("Open Markdown"),
        QString(), tr("Markdown files (*.md *.markdown *.txt);;All files (*)"));
    if (path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Open"), tr("Cannot open file:\n%1").arg(path));
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    m_editor->setPlainText(in.readAll());
    file.close();
    m_editor->document()->setModified(false);
    setCurrentFile(path);
    updatePreview();
}

void MainWindow::onOpenRecent()
{
    QAction *a = qobject_cast<QAction *>(sender());
    if (!a)
        return;
    QString path = a->data().toString();
    if (!QFile::exists(path)) {
        QMessageBox::warning(this, tr("Open Recent"), tr("File no longer exists:\n%1").arg(path));
        m_config->addRecentFile(path); // re-sorts; will be refreshed on next add
        m_config->clearRecentFiles();
        // rebuild a clean list without the missing file
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
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Open Recent"), tr("Cannot open:\n%1").arg(path));
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    m_editor->setPlainText(in.readAll());
    file.close();
    m_editor->document()->setModified(false);
    setCurrentFile(path);
    updatePreview();
}

void MainWindow::onSaveFile()
{
    if (m_currentFile.isEmpty())
        onSaveAs();
    else {
        saveFile(m_currentFile);
        m_editor->document()->setModified(false);
    }
}

void MainWindow::onSaveAs()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save Markdown"),
        m_currentFile.isEmpty() ? "untitled.md" : m_currentFile,
        tr("Markdown files (*.md);;All files (*)"));
    if (path.isEmpty())
        return;
    if (saveFile(path)) {
        setCurrentFile(path);
        m_editor->document()->setModified(false);
    }
}

bool MainWindow::saveFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save"), tr("Cannot save file:\n%1").arg(path));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << m_editor->toPlainText();
    file.close();
    statusBar()->showMessage(tr("Saved %1").arg(path), 3000);
    return true;
}

void MainWindow::onExportHtml()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export HTML"),
        m_currentFile.isEmpty() ? "output.html" : QFileInfo(m_currentFile).baseName() + ".html",
        tr("HTML files (*.html *.htm);;All files (*)"));
    if (path.isEmpty())
        return;
    MarkdownParser parser;
    QString body = parser.parse(m_editor->toPlainText());
    QString css = m_styleLoader->loadFromFile(m_config->cssFilePath());
    QString html = QString(
        "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
        "<title>%1</title><style>%2</style></head><body>%3</body></html>")
        .arg(QFileInfo(m_currentFile).baseName().toHtmlEscaped())
        .arg(css).arg(body);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export"), tr("Cannot write file:\n%1").arg(path));
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << html;
    file.close();
    statusBar()->showMessage(tr("Exported HTML: %1").arg(path), 3000);
}

void MainWindow::onExportPdf()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export PDF"),
        m_currentFile.isEmpty() ? "output.pdf" : QFileInfo(m_currentFile).baseName() + ".pdf",
        tr("PDF files (*.pdf);;All files (*)"));
    if (path.isEmpty())
        return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    m_preview->print(&printer);
    statusBar()->showMessage(tr("Exported PDF: %1").arg(path), 3000);
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
    static FindReplaceDialog *dlg = nullptr;
    if (!dlg)
        dlg = new FindReplaceDialog(m_editor, this);
    if (!dlg->isVisible()) {
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
    static FindReplaceDialog *dlg = nullptr;
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

void MainWindow::toggleLineNumbers(bool checked)
{
    m_config->setShowLineNumbers(checked);
    m_editor->setLineNumbersVisible(checked);
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
void MainWindow::formatBold()      { wrapSelection("**", "**"); }
void MainWindow::formatItalic()    { wrapSelection("*", "*"); }
void MainWindow::formatUnderline() { wrapSelection("<u>", "</u>"); }
void MainWindow::formatStrikethrough() { wrapSelection("~~", "~~"); }
void MainWindow::inlineCode()      { wrapSelection("`", "`"); }

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
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next()) {
        QString line = block.text();
        QString stripped = line;
        stripped.remove(QRegularExpression("^#{1,6}\\s+"));
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
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next()) {
        QString line = block.text();
        QString stripped = line;
        stripped.remove(QRegularExpression("^\\s*\\d+\\.\\s+"));
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
    if (cur.hasSelection()) {
        QString sel = cur.selectedText();
        cur.insertText("\n```\n" + sel + "\n```\n");
    } else {
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
    if (dlg.exec() == QDialog::Accepted) {
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
    if (dlg.exec() == QDialog::Accepted) {
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

// --------------------------------------------------------------------------
// Tools operations
// --------------------------------------------------------------------------
void MainWindow::selectCss()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select CSS Style"),
        m_config->cssFilePath(), tr("CSS files (*.css);;All files (*)"));
    if (path.isEmpty())
        return;
    m_config->setCssFilePath(path);
    m_config->saveConfig();
    QString css = m_styleLoader->loadFromFile(path);
    m_preview->setCSS(css);
}

void MainWindow::resetCss()
{
    m_config->setCssFilePath(":/styles/default.css");
    m_config->saveConfig();
    m_preview->setCSS(StyleSheetLoader::getDefaultCSS());
    statusBar()->showMessage(tr("CSS style reset to default"), 3000);
}

void MainWindow::openPreferences()
{
    SettingsDialog dlg(m_config, this);
    dlg.exec();
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
        "<h1>Markdown Syntax Guide</h1>"
        "<h2>Headings</h2><pre><code># H1\n## H2\n### H3</code></pre>"
        "<h2>Emphasis</h2><pre><code>**bold**  *italic*  ~~strike~~  `code`</code></pre>"
        "<h2>Lists</h2><pre><code>- item\n- item\n\n1. first\n2. second</code></pre>"
        "<h2>Block quote</h2><pre><code>&gt; quoted text</code></pre>"
        "<h2>Code block</h2><pre><code>```\ncode here\n```</code></pre>"
        "<h2>Links &amp; Images</h2><pre><code>[text](https://example.com)\n"
        "![alt](image.png)</code></pre>"
        "<h2>Horizontal rule</h2><pre><code>---</code></pre>";
    PreviewWidget *w = new PreviewWidget(this);
    w->setCSS(StyleSheetLoader::getDefaultCSS());
    w->setMarkdown(guide);
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Markdown Syntax Guide"));
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
    m_statusCursor->setText(tr("Line %1, Col %2").arg(line).arg(col));

    QString text = m_editor->toPlainText();
    int words = text.split(QRegularExpression("\\s+"), QString::SkipEmptyParts).count();
    m_statusWords->setText(tr("Words: %1").arg(words));
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
    QAction *undo = menu.addAction(tr("Undo"));
    connect(undo, &QAction::triggered, m_editor, &CodeEditor::undo);
    QAction *redo = menu.addAction(tr("Redo"));
    connect(redo, &QAction::triggered, m_editor, &CodeEditor::redo);
    menu.addSeparator();
    QAction *cut = menu.addAction(tr("Cut"));
    connect(cut, &QAction::triggered, m_editor, &CodeEditor::cut);
    QAction *copy = menu.addAction(tr("Copy"));
    connect(copy, &QAction::triggered, m_editor, &CodeEditor::copy);
    QAction *paste = menu.addAction(tr("Paste"));
    connect(paste, &QAction::triggered, m_editor, &CodeEditor::paste);
    menu.addSeparator();
    QAction *selAll = menu.addAction(tr("Select All"));
    connect(selAll, &QAction::triggered, this, &MainWindow::onSelectAll);
    menu.addSeparator();
    QAction *find = menu.addAction(tr("Find..."));
    connect(find, &QAction::triggered, this, &MainWindow::onFind);
    menu.addSeparator();
    QAction *bold = menu.addAction(tr("Bold"));
    connect(bold, &QAction::triggered, this, &MainWindow::formatBold);
    QAction *italic = menu.addAction(tr("Italic"));
    connect(italic, &QAction::triggered, this, &MainWindow::formatItalic);
    QAction *strike = menu.addAction(tr("Strikethrough"));
    connect(strike, &QAction::triggered, this, &MainWindow::formatStrikethrough);
    menu.addSeparator();
    QAction *link = menu.addAction(tr("Insert Link..."));
    connect(link, &QAction::triggered, this, &MainWindow::insertLink);
    QAction *img = menu.addAction(tr("Insert Image..."));
    connect(img, &QAction::triggered, this, &MainWindow::insertImage);
    menu.addSeparator();
    QAction *font = menu.addAction(tr("Font..."));
    connect(font, &QAction::triggered, this, &MainWindow::openPreferences);

    Q_UNUSED(pos);
    menu.exec(m_editor->viewport()->mapToGlobal(pos));
}

void MainWindow::previewContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *copy = menu.addAction(tr("Copy"));
    connect(copy, &QAction::triggered, this, [this]() {
        m_preview->triggerPageAction(QWebPage::Copy);
    });
    QAction *selAll = menu.addAction(tr("Select All"));
    connect(selAll, &QAction::triggered, this, [this]() {
        m_preview->triggerPageAction(QWebPage::SelectAll);
    });
    menu.exec(m_preview->mapToGlobal(pos));
}

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------
void MainWindow::wrapSelection(const QString &before, const QString &after)
{
    QTextCursor cur = m_editor->textCursor();
    if (cur.hasSelection()) {
        QString sel = cur.selectedText();
        int selStart = cur.selectionStart();
        if (sel.startsWith(before) && sel.endsWith(after) &&
            sel.length() >= before.length() + after.length()) {
            QString inner = sel.mid(before.length(),
                                    sel.length() - before.length() - after.length());
            cur.insertText(inner);
            cur.setPosition(selStart);
            cur.setPosition(selStart + inner.length(), QTextCursor::KeepAnchor);
        } else {
            cur.insertText(before + sel + after);
            cur.setPosition(selStart);
            cur.setPosition(selStart + before.length() + sel.length(), QTextCursor::KeepAnchor);
        }
    } else {
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
    for (; block.isValid() && block.position() <= endBlock.position(); block = block.next()) {
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
        tr("The document has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
        onSaveFile();
        if (m_editor->document()->isModified())
            return false;   // save was cancelled or failed
    }
    else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    QString title = APP_NAME;
    if (path.isEmpty())
        title = tr("Untitled") + " - " + title;
    else {
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
    if (recent.isEmpty()) {
        QAction *none = new QAction(tr("(no recent files)"), this);
        none->setEnabled(false);
        m_recentMenu->addAction(none);
        return;
    }
    for (const QString &f : recent) {
        QAction *a = new QAction(QFileInfo(f).fileName(), this);
        a->setData(f);
        a->setToolTip(f);
        connect(a, &QAction::triggered, this, &MainWindow::onOpenRecent);
        m_recentMenu->addAction(a);
    }
    m_recentMenu->addSeparator();
    QAction *clear = new QAction(tr("Clear List"), this);
    connect(clear, &QAction::triggered, this, [this]() {
        m_config->clearRecentFiles();
        m_config->saveConfig();
        updateRecentMenu();
    });
    m_recentMenu->addAction(clear);
}

// --------------------------------------------------------------------------
// Close event
// --------------------------------------------------------------------------
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_splitSet) {
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

// silence unused warnings for exit action
void MainWindow::onExit()
{
    close();
}
