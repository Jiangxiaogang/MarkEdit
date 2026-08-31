#include "insertdialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

InsertDialog::InsertDialog(const QString &kind, QWidget *parent)
    : QDialog(parent)
    , m_kind(kind)
{
    setModal(true);
    if (kind == "image")
        setWindowTitle(tr("插入图像"));
    else
        setWindowTitle(tr("插入链接"));

    m_textEdit = new QLineEdit(this);
    m_urlEdit = new QLineEdit(this);

    QPushButton *okBtn = new QPushButton(tr("确定"));
    QPushButton *cancelBtn = new QPushButton(tr("取消"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(okBtn);
    btnRow->addWidget(cancelBtn);

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Text:"), m_textEdit);
    form->addRow(tr("URL:"), m_urlEdit);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btnRow);

    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    m_textEdit->setFocus();
}

QString InsertDialog::text() const
{
    return m_textEdit->text();
}
QString InsertDialog::url() const
{
    return m_urlEdit->text();
}
void InsertDialog::setText(const QString &text)
{
    m_textEdit->setText(text);
}
