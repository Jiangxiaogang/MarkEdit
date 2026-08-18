#include "codeeditor.h"
#include <QTextBlock>
#include <QPainter>
#include <QKeyEvent>

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor), m_editor(editor)
{
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_showLineNumbers(true)
    , m_showWhitespace(false)
{
    // 连接信号槽
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    
    // 初始化
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    
    // 设置默认字体
    QFont font("Consolas", 12);
    font.setStyleHint(QFont::Monospace);
    setFont(font);
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::setLineNumbersVisible(bool visible)
{
    m_showLineNumbers = visible;
    updateLineNumberAreaWidth(0);
}

void CodeEditor::setWhitespaceVisible(bool visible)
{
    m_showWhitespace = visible;
    QTextOption option = document()->defaultTextOption();
    if (visible) {
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces | 
                       QTextOption::ShowLineAndParagraphSeparators);
    } else {
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces & 
                       ~QTextOption::ShowLineAndParagraphSeparators);
    }
    document()->setDefaultTextOption(option);
    viewport()->update();
}

void CodeEditor::setEditorFont(const QFont &font)
{
    setFont(font);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // Tab 键处理 - 插入空格而不是制表符
    if (event->key() == Qt::Key_Tab && !event->modifiers()) {
        insertPlainText("    "); // 4 个空格
        return;
    }
    
    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(Qt::lightGray);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (!m_showLineNumbers)
        return;
    
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);
    
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width() - 5, 
                           fontMetrics().height(), Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

int CodeEditor::lineNumberAreaWidth()
{
    if (!m_showLineNumbers)
        return 0;
    
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 10 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}
