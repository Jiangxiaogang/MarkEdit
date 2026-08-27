#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QFrame>

class LineNumberArea;
class MarkdownHighlighter;

/**
 * @brief A Markdown source editor based on QPlainTextEdit.
 *
 * Adds:
 *   - a line-number gutter (LineNumberArea)
 *   - current-line highlighting
 *   - optional visible whitespace characters (tabs / spaces / paragraph marks)
 *   - configurable font
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return m_showLineNumbers; }

    void setWhitespaceVisible(bool visible);
    bool whitespaceVisible() const { return m_showWhitespace; }

    void setSyntaxHighlightingEnabled(bool enabled);

    void setTabWidth(int width);

    void setEditorFont(const QFont &font);

    int lineNumberAreaWidth() const;

    // Called by LineNumberArea
    void lineNumberAreaPaintEvent(QPaintEvent *event);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
    MarkdownHighlighter *m_highlighter;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    bool m_syntaxHighlighting;
};

#endif // CODEEDITOR_H
