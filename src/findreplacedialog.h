#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class QPlainTextEdit;

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
    QLineEdit *m_findEdit;
    QLineEdit *m_replaceEdit;
    QCheckBox *m_caseCheck;
    QCheckBox *m_wholeWordCheck;
    QPushButton *m_replaceBtn;
    QPushButton *m_replaceAllBtn;

    bool find(bool forward);
};

#endif // FINDREPLACEDIALOG_H
