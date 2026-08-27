#ifndef INSERTDIALOG_H
#define INSERTDIALOG_H

#include <QDialog>

/**
 * @brief Small prompt used to insert Markdown links and images.
 */
class InsertDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief kind selects the dialog mode ("link" or "image").
     */
    explicit InsertDialog(const QString &kind, QWidget *parent = nullptr);

    QString text() const;
    QString url() const;
    void setText(const QString &text);

private:
    class QLineEdit *m_textEdit;
    class QLineEdit *m_urlEdit;
    QString m_kind;
};

#endif // INSERTDIALOG_H
