#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

class QPlainTextEdit;

/**
 * @brief Modal find / replace dialog operating on a QPlainTextEdit.
 */
class FindReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent = 0);
    void setFindText(const QString &text);

private slots:
    void findNext();
    void replaceOne();
    void replaceAll();
    void updateButtons();

private:
    QPlainTextEdit *m_editor;
    class QLineEdit *m_findEdit;
    class QLineEdit *m_replaceEdit;
    class QCheckBox *m_caseCheck;
    class QCheckBox *m_wholeWordCheck;
    class QPushButton *m_replaceBtn;
    class QPushButton *m_replaceAllBtn;

    bool find(bool forward);
};

#endif // FINDREPLACEDIALOG_H
