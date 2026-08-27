#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

class CodeEditor;

/**
 * @brief A small widget painted in the left margin of a CodeEditor that
 *        displays line numbers. It is a passive view driven by CodeEditor.
 */
class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_codeEditor;
};

#endif // LINENUMBERAREA_H
