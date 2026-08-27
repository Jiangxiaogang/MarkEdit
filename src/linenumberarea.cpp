#include "linenumberarea.h"
#include "codeeditor.h"

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , m_codeEditor(editor)
{
    setObjectName("lineNumberArea");
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_codeEditor->lineNumberAreaPaintEvent(event);
}
