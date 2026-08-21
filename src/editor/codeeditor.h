#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>

class CodeEditor;

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor);
    
protected:
    void paintEvent(QPaintEvent *event);
    
private:
    CodeEditor *m_editor;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
    
public:
    explicit CodeEditor(QWidget *parent = 0);
    ~CodeEditor();
    
    void setLineNumbersVisible(bool visible);
    void setWhitespaceVisible(bool visible);
    void setEditorFont(const QFont &font);
    
signals:
    void cursorPositionChanged();
    
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    
private:
    QWidget *m_lineNumberArea;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    
    friend class LineNumberArea;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
};

#endif // CODEEDITOR_H
