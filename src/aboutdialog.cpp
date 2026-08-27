#include "aboutdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSysInfo>
#include <qglobal.h>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About MarkEdit"));
    setModal(true);

    QLabel *title = new QLabel(QString("<h2>MarkEdit</h2>"));
    title->setAlignment(Qt::AlignCenter);

    QLabel *version = new QLabel(QString("Version %1").arg("1.0.0"));
    version->setAlignment(Qt::AlignCenter);

    QString info = QString(
        "<p align='center'>A cross-platform Markdown editor built with Qt %1.</p>"
        "<p align='center'>Copyright &copy; 2026 MarkEdit contributors.<br>"
        "Licensed under the MIT License.</p>")
        .arg(QT_VERSION_STR);

    QLabel *details = new QLabel(info);
    details->setWordWrap(true);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(title);
    main->addWidget(version);
    main->addWidget(details);
    main->addWidget(box);
}
