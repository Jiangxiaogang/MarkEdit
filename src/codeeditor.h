#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFrame>

class LineNumberArea;
class MarkdownHighlighter;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = 0);
    ~CodeEditor();

    void setLineWrapEnabled(bool enabled);
    void setLineNumberVisible(bool visible);
    void setWhitespaceVisible(bool visible);
    void setSyntaxHighlightingEnabled(bool enabled);
    void setTabWidth(int width);
    void setEditorFont(const QFont &font);
    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
    MarkdownHighlighter *m_highlighter;
    bool m_lineWarp;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    bool m_syntaxHighlighting;
};

#endif // CODEEDITOR_H
