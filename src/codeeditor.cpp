#include "codeeditor.h"
#include "linenumberarea.h"
#include "markdownhighlighter.h"

#include <QPainter>
#include <QTextBlock>
#include <QTextOption>
#include <QRect>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextCursor>
#include <QTextCharFormat>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
    , m_highlighter(new MarkdownHighlighter(document()))
    , m_showLineNumbers(true)
    , m_showWhitespace(false)
    , m_syntaxHighlighting(true)
{
    setObjectName("codeEditor");
    setFrameShape(QFrame::NoFrame);   // remove the editor's border

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // Apply whitespace visibility according to current flag
    QTextOption option = document()->defaultTextOption();
    option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    document()->setDefaultTextOption(option);
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::setLineNumbersVisible(bool visible)
{
    m_showLineNumbers = visible;
    m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
}

void CodeEditor::setWhitespaceVisible(bool visible)
{
    m_showWhitespace = visible;
    QTextOption option = document()->defaultTextOption();
    if (visible)
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces |
                        QTextOption::ShowLineAndParagraphSeparators);
    else
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces &
                        ~QTextOption::ShowLineAndParagraphSeparators);
    document()->setDefaultTextOption(option);
    viewport()->update();
}

void CodeEditor::setSyntaxHighlightingEnabled(bool enabled)
{
    m_syntaxHighlighting = enabled;
    if (enabled) {
        m_highlighter->setDocument(document());
    } else {
        m_highlighter->setDocument(nullptr);
        // Remove any colours already applied by the highlighter.
        QTextCursor cursor(document());
        cursor.select(QTextCursor::Document);
        cursor.setCharFormat(QTextCharFormat());
    }
}

void CodeEditor::setTabWidth(int width)
{
    // QPlainTextEdit's tab stop is expressed in pixels. The configured value
    // is a number of space characters, so scale it by the space width.
    setTabStopWidth(width * fontMetrics().width(QLatin1Char(' ')));
}

void CodeEditor::setEditorFont(const QFont &font)
{
    setFont(font);
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_showLineNumbers)
        return 0;
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (m_showLineNumbers) {
        if (dy)
            m_lineNumberArea->scroll(0, dy);
        else
            m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(60, 60, 60, 30);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPlainTextEdit::paintEvent(event);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (!m_showLineNumbers)
        return;

    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, m_lineNumberArea->width() - 3,
                             fontMetrics().height(), Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
