#ifndef INSERTDIALOG_H
#define INSERTDIALOG_H

#include <QDialog>

class InsertDialog : public QDialog
{
    Q_OBJECT
public:
    explicit InsertDialog(const QString &kind, QWidget *parent = 0);

    QString text() const;
    QString url() const;
    void setText(const QString &text);

private:
    class QLineEdit *m_textEdit;
    class QLineEdit *m_urlEdit;
    QString m_kind;
};

#endif // INSERTDIALOG_H
