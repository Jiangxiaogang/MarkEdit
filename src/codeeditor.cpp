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
    , m_currentLineColor(QColor(60, 60, 60, 30))
{
    setObjectName("codeEditor");
    setFrameShape(QFrame::NoFrame);

    connect(this, SIGNAL(blockCountChanged(int)),this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)),this, SLOT(updateLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()),this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    setLineWrapEnabled(false);
}

CodeEditor::~CodeEditor()
{
}

void CodeEditor::setLineWrapEnabled(bool enabled)
{
    m_lineWarp = enabled;
    setLineWrapMode(m_lineWarp ? WidgetWidth : NoWrap);
}

void CodeEditor::setLineNumberVisible(bool visible)
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
        option.setFlags(option.flags() | QTextOption::ShowTabsAndSpaces);
    else
        option.setFlags(option.flags() & ~QTextOption::ShowTabsAndSpaces &
                        ~QTextOption::ShowLineAndParagraphSeparators);
    document()->setDefaultTextOption(option);
    viewport()->update();
}

void CodeEditor::setSyntaxHighlightingEnabled(bool enabled)
{
    m_syntaxHighlighting = enabled;
    if (enabled)
    {
        m_highlighter->setDocument(document());
    }
    else
    {
        m_highlighter->setDocument(0);
        QTextCursor cursor(document());
        cursor.select(QTextCursor::Document);
        cursor.setCharFormat(QTextCharFormat());
    }
}

void CodeEditor::setTabWidth(int width)
{
    setTabStopWidth(width * fontMetrics().width(QLatin1Char(' ')));
}

void CodeEditor::setEditorFont(const QFont &font)
{
    setFont(font);
}

void CodeEditor::setCurrentLineColor(const QColor &color)
{
    m_currentLineColor = color;
    highlightCurrentLine();
}

void CodeEditor::setSelectionColor(const QColor &color)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Highlight, color);
    // Keep selected text readable on the custom highlight colour.
    pal.setColor(QPalette::HighlightedText, Qt::white);
    setPalette(pal);
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (m_showLineNumbers)
    {
        int max = qMax(1, blockCount());
        int space = fontMetrics().width(QString::number(max * 10));
        return space;
    }
    return 0;
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (m_showLineNumbers)
    {
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
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_currentLineColor);
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

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
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
