#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QCloseEvent>
#include <QTextStream>
#include <QPrinter>
#include <QPrintDialog>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_parser(nullptr)
    , m_config(nullptr)
    , m_updateTimer(nullptr)
    , m_isModified(false)
{
    ui->setupUi(this);
    
    // 初始化配置管理器
    m_config = ConfigManager::instance();
    
    // 初始化解析器
    m_parser = new MarkdownParser(this);
    
    initUI();
    initMenuBar();
    initConnections();
    loadSettings();
    updateMenusState();
    
    // 设置窗口标题
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

void MainWindow::initUI()
{
    // 创建主分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建编辑器
    m_editor = new CodeEditor(this);
    
    // 创建预览区
    m_preview = new PreviewWidget(this);
    
    // 添加到分割器
    m_splitter->addWidget(m_editor);
    m_splitter->addWidget(m_preview);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes(QList<int>() << 400 << 400);
    
    setCentralWidget(m_splitter);
    
    // 创建状态栏
    m_statusLabel = new QLabel(tr("Ready"));
    m_cursorPosLabel = new QLabel(tr("Line: 1, Col: 1"));
    m_wordCountLabel = new QLabel(tr("Words: 0"));
    
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_cursorPosLabel);
    statusBar()->addPermanentWidget(m_wordCountLabel);
    
    // 创建延迟更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(300); // 300ms 防抖
}

void MainWindow::initMenuBar()
{
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createFormatMenu();
    createToolsMenu();
    createHelpMenu();
}

void MainWindow::createFileMenu()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    
    m_actNew = m_fileMenu->addAction(tr("&New"), this, &MainWindow::newFile);
    m_actNew->setShortcut(QKeySequence::New);
    
    m_actOpen = m_fileMenu->addAction(tr("&Open..."), this, &MainWindow::openFile);
    m_actOpen->setShortcut(QKeySequence::Open);
    
    m_fileMenu->addSeparator();
    
    m_actSave = m_fileMenu->addAction(tr("&Save"), this, &MainWindow::saveFile);
    m_actSave->setShortcut(QKeySequence::Save);
    
    m_actSaveAs = m_fileMenu->addAction(tr("Save &As..."), this, &MainWindow::saveFileAs);
    m_actSaveAs->setShortcut(QKeySequence::SaveAs);
    
    m_fileMenu->addSeparator();
    
    m_actExportHtml = m_fileMenu->addAction(tr("Export to &HTML"), this, &MainWindow::exportToHtml);
    m_actExportPdf = m_fileMenu->addAction(tr("Export to &PDF"), this, &MainWindow::exportToPdf);
    m_actPrint = m_fileMenu->addAction(tr("&Print..."), this, &MainWindow::printFile);
    m_actPrint->setShortcut(QKeySequence::Print);
    
    m_fileMenu->addSeparator();
    
    m_actExit = m_fileMenu->addAction(tr("E&xit"), this, &MainWindow::exitApp);
    m_actExit->setShortcut(QKeySequence::Quit);
}

void MainWindow::createEditMenu()
{
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    
    m_actUndo = m_editMenu->addAction(tr("&Undo"), this, &MainWindow::undo);
    m_actUndo->setShortcut(QKeySequence::Undo);
    
    m_actRedo = m_editMenu->addAction(tr("&Redo"), this, &MainWindow::redo);
    m_actRedo->setShortcut(QKeySequence::Redo);
    
    m_editMenu->addSeparator();
    
    m_actCut = m_editMenu->addAction(tr("Cu&t"), this, &MainWindow::cut);
    m_actCut->setShortcut(QKeySequence::Cut);
    
    m_actCopy = m_editMenu->addAction(tr("&Copy"), this, &MainWindow::copy);
    m_actCopy->setShortcut(QKeySequence::Copy);
    
    m_actPaste = m_editMenu->addAction(tr("&Paste"), this, &MainWindow::paste);
    m_actPaste->setShortcut(QKeySequence::Paste);
    
    m_editMenu->addSeparator();
    
    m_actSelectAll = m_editMenu->addAction(tr("Select &All"), this, &MainWindow::selectAll);
    m_actSelectAll->setShortcut(QKeySequence::SelectAll);
    
    m_editMenu->addSeparator();
    
    m_actFindReplace = m_editMenu->addAction(tr("&Find/Replace"), this, &MainWindow::findReplace);
    m_actFindReplace->setShortcut(QKeySequence::Find);
    
    m_actGoToLine = m_editMenu->addAction(tr("&Go to Line..."), this, &MainWindow::goToLine);
    m_actGoToLine->setShortcut(tr("Ctrl+G"));
}

void MainWindow::createViewMenu()
{
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    
    m_actLineNumbers = m_viewMenu->addAction(tr("&Line Numbers"), this, &MainWindow::toggleLineNumbers);
    m_actLineNumbers->setCheckable(true);
    m_actLineNumbers->setChecked(true);
    
    m_actWhitespace = m_viewMenu->addAction(tr("&Whitespace Characters"), this, &MainWindow::toggleWhitespace);
    m_actWhitespace->setCheckable(true);
    m_actWhitespace->setChecked(false);
    
    m_viewMenu->addSeparator();
    
    m_actFullScreen = m_viewMenu->addAction(tr("&Full Screen"), this, &MainWindow::toggleFullScreen);
    m_actFullScreen->setCheckable(true);
    m_actFullScreen->setShortcut(tr("F11"));
    
    m_actPreviewPane = m_viewMenu->addAction(tr("&Preview Pane"), this, &MainWindow::togglePreviewPane);
    m_actPreviewPane->setCheckable(true);
    m_actPreviewPane->setChecked(true);
    
    m_viewMenu->addSeparator();
    
    m_actSplitH = m_viewMenu->addAction(tr("Split &Horizontal"), this, &MainWindow::splitHorizontally);
    m_actSplitH->setCheckable(true);
    m_actSplitH->setChecked(true);
    
    m_actSplitV = m_viewMenu->addAction(tr("Split &Vertical"), this, &MainWindow::splitVertically);
    m_actSplitV->setCheckable(true);
    
    m_actResetLayout = m_viewMenu->addAction(tr("&Reset Layout"), this, &MainWindow::resetLayout);
    
    m_viewMenu->addSeparator();
    
    m_actZoomIn = m_viewMenu->addAction(tr("Zoom &In"), this, &MainWindow::zoomIn);
    m_actZoomIn->setShortcut(QKeySequence::ZoomIn);
    
    m_actZoomOut = m_viewMenu->addAction(tr("Zoom &Out"), this, &MainWindow::zoomOut);
    m_actZoomOut->setShortcut(QKeySequence::ZoomOut);
    
    m_actResetZoom = m_viewMenu->addAction(tr("&Reset Zoom"), this, &MainWindow::resetZoom);
    m_actResetZoom->setShortcut(tr("Ctrl+0"));
}

void MainWindow::createFormatMenu()
{
    m_formatMenu = menuBar()->addMenu(tr("F&ormat"));
    
    m_actBold = m_formatMenu->addAction(tr("&Bold"), this, &MainWindow::formatBold);
    m_actBold->setShortcut(tr("Ctrl+B"));
    
    m_actItalic = m_formatMenu->addAction(tr("&Italic"), this, &MainWindow::formatItalic);
    m_actItalic->setShortcut(tr("Ctrl+I"));
    
    m_actUnderline = m_formatMenu->addAction(tr("&Underline"), this, &MainWindow::formatUnderline);
    m_actUnderline->setShortcut(tr("Ctrl+U"));
    
    m_actStrike = m_formatMenu->addAction(tr("&Strikethrough"), this, &MainWindow::formatStrikethrough);
    
    m_formatMenu->addSeparator();
    
    m_actInlineCode = m_formatMenu->addAction(tr("&Inline Code"), this, &MainWindow::formatInlineCode);
    m_actInlineCode->setShortcut(tr("Ctrl+`"));
    
    m_formatMenu->addSeparator();
    
    m_actH1 = m_formatMenu->addAction(tr("Header &1"), this, &MainWindow::formatHeader1);
    m_actH1->setShortcut(tr("Ctrl+1"));
    
    m_actH2 = m_formatMenu->addAction(tr("Header &2"), this, &MainWindow::formatHeader2);
    m_actH2->setShortcut(tr("Ctrl+2"));
    
    m_actH3 = m_formatMenu->addAction(tr("Header &3"), this, &MainWindow::formatHeader3);
    m_actH3->setShortcut(tr("Ctrl+3"));
    
    m_formatMenu->addSeparator();
    
    m_actBulletList = m_formatMenu->addAction(tr("&Bullet List"), this, &MainWindow::formatBulletList);
    m_actBulletList->setShortcut(tr("Ctrl+Shift+B"));
    
    m_actNumberedList = m_formatMenu->addAction(tr("&Numbered List"), this, &MainWindow::formatNumberedList);
    m_actNumberedList->setShortcut(tr("Ctrl+Shift+N"));
    
    m_actBlockquote = m_formatMenu->addAction(tr("&Blockquote"), this, &MainWindow::formatBlockquote);
    m_actBlockquote->setShortcut(tr("Ctrl+Shift+Q"));
    
    m_actCodeBlock = m_formatMenu->addAction(tr("C&ode Block"), this, &MainWindow::formatCodeBlock);
    m_actCodeBlock->setShortcut(tr("Ctrl+Shift+C"));
    
    m_formatMenu->addSeparator();
    
    m_actLink = m_formatMenu->addAction(tr("&Link"), this, &MainWindow::formatLink);
    m_actLink->setShortcut(tr("Ctrl+K"));
    
    m_actImage = m_formatMenu->addAction(tr("&Image"), this, &MainWindow::formatImage);
    m_actImage->setShortcut(tr("Ctrl+Shift+I"));
    
    m_actHr = m_formatMenu->addAction(tr("&Horizontal Rule"), this, &MainWindow::formatHorizontalRule);
}

void MainWindow::createToolsMenu()
{
    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    
    m_actSettings = m_toolsMenu->addAction(tr("&Settings..."), this, &MainWindow::showSettings);
    m_actSettings->setShortcut(tr("Ctrl+,"));
    
    m_toolsMenu->addSeparator();
    
    m_actLoadCss = m_toolsMenu->addAction(tr("&Load CSS File..."), this, &MainWindow::loadCssFile);
    m_actResetCss = m_toolsMenu->addAction(tr("&Reset CSS"), this, &MainWindow::resetCss);
}

void MainWindow::createHelpMenu()
{
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    
    m_actAbout = m_helpMenu->addAction(tr("&About"), this, &MainWindow::aboutApp);
    
    m_helpMenu->addSeparator();
    
    m_actAboutQt = m_helpMenu->addAction(tr("About &Qt"), this, &MainWindow::aboutQt);
}

void MainWindow::initConnections()
{
    // 文本变化连接
    connect(m_editor, &CodeEditor::textChanged, this, [this]() {
        m_updateTimer->start();
        m_isModified = true;
        updateWindowTitle();
        updateStatusBar();
    });
    
    // 延迟更新预览
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        QString markdown = m_editor->toPlainText();
        m_preview->updatePreview(markdown);
        m_isModified = false;
    });
    
    // 光标位置变化
    connect(m_editor, &CodeEditor::cursorPositionChanged, this, &MainWindow::updateStatusBar);
}

void MainWindow::loadSettings()
{
    // 加载编辑器设置
    m_editor->setLineNumbersVisible(m_config->showLineNumbers());
    m_editor->setWhitespaceVisible(m_config->showWhitespace());
    m_editor->setEditorFont(m_config->editorFont());
    
    // 加载 CSS 文件
    QString cssPath = m_config->cssFilePath();
    if (!cssPath.isEmpty()) {
        m_preview->loadCSSFile(cssPath);
    }
    
    // 恢复窗口布局
    QList<int> sizes = m_config->splitterSizes();
    if (!sizes.isEmpty()) {
        m_splitter->setSizes(sizes);
    }
    
    // 设置菜单状态
    m_actLineNumbers->setChecked(m_config->showLineNumbers());
    m_actWhitespace->setChecked(m_config->showWhitespace());
}

void MainWindow::saveSettings()
{
    // 保存编辑器设置
    m_config->setShowLineNumbers(m_actLineNumbers->isChecked());
    m_config->setShowWhitespace(m_actWhitespace->isChecked());
    m_config->setEditorFont(m_editor->font());
    
    // 保存窗口布局
    m_config->setSplitterSizes(m_splitter->sizes());
    
    // 保存配置
    m_config->saveConfig();
}

void MainWindow::updateMenusState()
{
    bool hasText = !m_editor->toPlainText().isEmpty();
    bool hasSelection = !m_editor->textCursor().selectedText().isEmpty();
    
    m_actUndo->setEnabled(m_editor->document()->isUndoAvailable());
    m_actRedo->setEnabled(m_editor->document()->isRedoAvailable());
    m_actCut->setEnabled(hasSelection);
    m_actCopy->setEnabled(hasSelection);
    m_actPaste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    m_actSave->setEnabled(m_isModified);
}

// 文件菜单槽函数
void MainWindow::newFile()
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Save Changes"),
            tr("Do you want to save changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveFile();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    m_editor->clear();
    m_currentFilePath.clear();
    m_isModified = false;
    updateWindowTitle();
    m_statusLabel->setText(tr("New file created"));
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "",
        tr("Markdown Files (*.md *.markdown);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file: %1").arg(fileName));
        return;
    }
    
    QTextStream in(&file);
    m_editor->setPlainText(in.readAll());
    file.close();
    
    m_currentFilePath = fileName;
    m_isModified = false;
    updateWindowTitle();
    m_statusLabel->setText(tr("File opened: %1").arg(fileName));
}

void MainWindow::saveFile()
{
    if (m_currentFilePath.isEmpty()) {
        saveFileAs();
        return;
    }
    
    QFile file(m_currentFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot save file: %1").arg(m_currentFilePath));
        return;
    }
    
    QTextStream out(&file);
    out << m_editor->toPlainText();
    file.close();
    
    m_isModified = false;
    updateWindowTitle();
    m_statusLabel->setText(tr("File saved: %1").arg(m_currentFilePath));
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "",
        tr("Markdown Files (*.md *.markdown);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;
    
    m_currentFilePath = fileName;
    saveFile();
}

void MainWindow::exportToHtml()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to HTML"), "",
        tr("HTML Files (*.html);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;
    
    QString markdown = m_editor->toPlainText();
    QString html = m_parser->parse(markdown);
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << html;
        file.close();
        m_statusLabel->setText(tr("Exported to HTML: %1").arg(fileName));
    }
}

void MainWindow::exportToPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to PDF"), "",
        tr("PDF Files (*.pdf);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;
    
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    
    QTextDocument doc;
    doc.setHtml(m_preview->toHtml());
    doc.print(&printer);
    
    m_statusLabel->setText(tr("Exported to PDF: %1").arg(fileName));
}

void MainWindow::printFile()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QTextDocument doc;
        doc.setHtml(m_preview->toHtml());
        doc.print(&printer);
        m_statusLabel->setText(tr("File printed"));
    }
}

void MainWindow::exitApp()
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Save Changes"),
            tr("Do you want to save changes before exiting?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveFile();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    
    saveSettings();
    qApp->quit();
}

// 编辑菜单槽函数
void MainWindow::undo()
{
    m_editor->undo();
}

void MainWindow::redo()
{
    m_editor->redo();
}

void MainWindow::cut()
{
    m_editor->cut();
}

void MainWindow::copy()
{
    m_editor->copy();
}

void MainWindow::paste()
{
    m_editor->paste();
}

void MainWindow::selectAll()
{
    m_editor->selectAll();
}

void MainWindow::findReplace()
{
    // TODO: 实现查找替换对话框
    m_statusLabel->setText(tr("Find/Replace not implemented yet"));
}

void MainWindow::goToLine()
{
    // TODO: 实现跳转到行功能
    m_statusLabel->setText(tr("Go to Line not implemented yet"));
}

// 视图菜单槽函数
void MainWindow::toggleLineNumbers(bool checked)
{
    m_editor->setLineNumbersVisible(checked);
    m_config->setShowLineNumbers(checked);
}

void MainWindow::toggleWhitespace(bool checked)
{
    m_editor->setWhitespaceVisible(checked);
    m_config->setShowWhitespace(checked);
}

void MainWindow::toggleFullScreen(bool checked)
{
    if (checked) {
        showFullScreen();
    } else {
        showNormal();
    }
}

void MainWindow::togglePreviewPane(bool checked)
{
    m_preview->setVisible(checked);
}

void MainWindow::splitHorizontally(bool checked)
{
    if (checked) {
        m_splitter->setOrientation(Qt::Horizontal);
        m_actSplitV->setChecked(false);
    }
}

void MainWindow::splitVertically(bool checked)
{
    if (checked) {
        m_splitter->setOrientation(Qt::Vertical);
        m_actSplitH->setChecked(false);
    }
}

void MainWindow::resetLayout()
{
    m_splitter->setOrientation(Qt::Horizontal);
    m_splitter->setSizes(QList<int>() << 400 << 400);
    m_actSplitH->setChecked(true);
    m_actSplitV->setChecked(false);
    m_actPreviewPane->setChecked(true);
    m_preview->setVisible(true);
}

void MainWindow::zoomIn()
{
    QFont font = m_editor->font();
    font.setPointSize(font.pointSize() + 1);
    m_editor->setFont(font);
    m_preview->setFont(font);
}

void MainWindow::zoomOut()
{
    QFont font = m_editor->font();
    if (font.pointSize() > 6) {
        font.setPointSize(font.pointSize() - 1);
        m_editor->setFont(font);
        m_preview->setFont(font);
    }
}

void MainWindow::resetZoom()
{
    QFont font = m_config->editorFont();
    m_editor->setFont(font);
    m_preview->setFont(font);
}

// 格式菜单槽函数
void MainWindow::formatBold()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("**" + cursor.selectedText() + "**");
    } else {
        cursor.insertText("****");
        cursor.setPosition(cursor.position() - 2);
        m_editor->setTextCursor(cursor);
    }
}

void MainWindow::formatItalic()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("*" + cursor.selectedText() + "*");
    } else {
        cursor.insertText("**");
        cursor.setPosition(cursor.position() - 1);
        m_editor->setTextCursor(cursor);
    }
}

void MainWindow::formatUnderline()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("<u>" + cursor.selectedText() + "</u>");
    }
}

void MainWindow::formatStrikethrough()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("~~" + cursor.selectedText() + "~~");
    } else {
        cursor.insertText("~~~~");
        cursor.setPosition(cursor.position() - 2);
        m_editor->setTextCursor(cursor);
    }
}

void MainWindow::formatInlineCode()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("`" + cursor.selectedText() + "`");
    } else {
        cursor.insertText("``");
        cursor.setPosition(cursor.position() - 1);
        m_editor->setTextCursor(cursor);
    }
}

void MainWindow::formatHeader1()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.insertText("# " + cursor.selectedText());
}

void MainWindow::formatHeader2()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.insertText("## " + cursor.selectedText());
}

void MainWindow::formatHeader3()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.insertText("### " + cursor.selectedText());
}

void MainWindow::formatBulletList()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("- ");
}

void MainWindow::formatNumberedList()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("1. ");
}

void MainWindow::formatBlockquote()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("> ");
}

void MainWindow::formatCodeBlock()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("\n```\n\n```\n");
    cursor.setPosition(cursor.position() - 4);
    m_editor->setTextCursor(cursor);
}

void MainWindow::formatLink()
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.insertText("[" + cursor.selectedText() + "](url)");
        cursor.setPosition(cursor.position() - 5);
        m_editor->setTextCursor(cursor);
    } else {
        cursor.insertText("[text](url)");
        cursor.setPosition(cursor.position() - 9);
        m_editor->setTextCursor(cursor);
    }
}

void MainWindow::formatImage()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("![alt text](image.png)");
    cursor.setPosition(cursor.position() - 15);
    m_editor->setTextCursor(cursor);
}

void MainWindow::formatHorizontalRule()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.insertText("\n---\n");
}

// 工具菜单槽函数
void MainWindow::showSettings()
{
    // TODO: 实现设置对话框
    m_statusLabel->setText(tr("Settings not implemented yet"));
}

void MainWindow::loadCssFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load CSS File"), "",
        tr("CSS Files (*.css);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;
    
    m_preview->loadCSSFile(fileName);
    m_config->setCssFilePath(fileName);
    m_statusLabel->setText(tr("CSS loaded: %1").arg(fileName));
}

void MainWindow::resetCss()
{
    m_preview->resetToDefaultCSS();
    m_config->setCssFilePath("");
    m_statusLabel->setText(tr("CSS reset to default"));
}

// 帮助菜单槽函数
void MainWindow::aboutApp()
{
    QMessageBox::about(this, tr("About Markdown Editor"),
        tr("<h2>Markdown Editor</h2>"
           "<p>Version 1.0</p>"
           "<p>A simple Markdown editor based on Qt5.12</p>"
           "<p>Features:</p>"
           "<ul>"
           "<li>Real-time preview</li>"
           "<li>Line numbers</li>"
           "<li>Custom CSS styling</li>"
           "<li>Export to HTML/PDF</li>"
           "</ul>"));
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

// 其他槽函数
void MainWindow::onTextChanged()
{
    updateStatusBar();
}

void MainWindow::updateWindowTitle()
{
    QString title = tr("Markdown Editor");
    if (!m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title = fileInfo.fileName() + " - " + title;
    }
    if (m_isModified) {
        title = "* " + title;
    }
    setWindowTitle(title);
}

void MainWindow::updateStatusBar()
{
    QTextCursor cursor = m_editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.columnNumber() + 1;
    m_cursorPosLabel->setText(tr("Line: %1, Col: %2").arg(line).arg(col));
    
    QString text = m_editor->toPlainText();
    int wordCount = text.split(QRegExp("\\s+"), Qt::SkipEmptyParts).size();
    m_wordCountLabel->setText(tr("Words: %1").arg(wordCount));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Save Changes"),
            tr("Do you want to save changes before closing?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            saveFile();
            event->accept();
        } else if (reply == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}
