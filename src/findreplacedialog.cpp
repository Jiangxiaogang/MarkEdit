#include "findreplacedialog.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextCursor>
#include <QRegExp>

FindReplaceDialog::FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
{
    setWindowTitle(tr("查找替换"));
    setModal(true);

    m_findEdit = new QLineEdit(this);
    m_replaceEdit = new QLineEdit(this);
    m_caseCheck = new QCheckBox(tr("区分大小写"), this);
    m_wholeWordCheck = new QCheckBox(tr("全字匹配"), this);

    QPushButton *findBtn = new QPushButton(tr("查找下一个"), this);
    m_replaceBtn = new QPushButton(tr("替换"), this);
    m_replaceAllBtn = new QPushButton(tr("全部替换"), this);
    QPushButton *closeBtn = new QPushButton(tr("关闭"), this);

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("查找:"), m_findEdit);
    form->addRow(tr("替换为:"), m_replaceEdit);
    form->addRow(m_caseCheck);
    form->addRow(m_wholeWordCheck);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(findBtn);
    btnRow->addWidget(m_replaceBtn);
    btnRow->addWidget(m_replaceAllBtn);
    btnRow->addWidget(closeBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addLayout(btnRow);

    connect(m_findEdit, SIGNAL(textChanged(QString)), this, SLOT(updateButtons()));
    connect(findBtn, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(m_replaceBtn, SIGNAL(clicked()), this, SLOT(replaceOne()));
    connect(m_replaceAllBtn, SIGNAL(clicked()), this, SLOT(replaceAll()));
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(close()));

    updateButtons();
    m_findEdit->setFocus();
}

void FindReplaceDialog::updateButtons()
{
    bool has = !m_findEdit->text().isEmpty();
    m_replaceBtn->setEnabled(has);
    m_replaceAllBtn->setEnabled(has);
}

void FindReplaceDialog::setFindText(const QString &text)
{
    m_findEdit->setText(text);
    m_findEdit->selectAll();
}

bool FindReplaceDialog::find(bool /*forward*/)
{
    QString term = m_findEdit->text();
    if (term.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;

    // Start the search right after the current selection / cursor.
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection())
        cursor.setPosition(cursor.selectionEnd());

    QTextCursor found = m_editor->document()->find(term, cursor, flags);
    if (found.isNull())
    {
        // wrap around from the beginning
        QTextCursor start(m_editor->document());
        start.movePosition(QTextCursor::Start);
        found = m_editor->document()->find(term, start, flags);
    }
    if (!found.isNull())
    {
        m_editor->setTextCursor(found);
        return true;
    }
    return false;
}

void FindReplaceDialog::findNext()
{
    find(true);
}

void FindReplaceDialog::replaceOne()
{
    QString term = m_findEdit->text();
    QString replacement = m_replaceEdit->text();
    if (term.isEmpty())
        return;

    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection() || cursor.selectedText() != term)
    {
        if (!find(true))
            return;
        cursor = m_editor->textCursor();
    }
    cursor.insertText(replacement);
    find(true);
}

void FindReplaceDialog::replaceAll()
{
    QString term = m_findEdit->text();
    QString replacement = m_replaceEdit->text();
    if (term.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;

    QTextCursor cursor(m_editor->document());
    cursor.movePosition(QTextCursor::Start);
    int count = 0;
    for (;;)
    {
        QTextCursor found = m_editor->document()->find(term, cursor, flags);
        if (found.isNull())
            break;
        found.insertText(replacement);
        cursor = found;
        cursor.movePosition(QTextCursor::EndOfWord);
        ++count;
    }
    if (count == 0)
        return;
}
