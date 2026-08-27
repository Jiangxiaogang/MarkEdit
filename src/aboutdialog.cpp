#include "aboutdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSysInfo>
#include <qglobal.h>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("关于 MarkEdit"));
    setModal(true);

    QLabel *title = new QLabel(QString("<h2>MarkEdit</h2>"));
    title->setAlignment(Qt::AlignCenter);

    QLabel *version = new QLabel(QString("版本 %1").arg("1.0.0"));
    version->setAlignment(Qt::AlignCenter);

    QString info = QString(
        "<p align='center'>一个基于 Qt %1 的跨平台 Markdown 编辑器。</p>"
        "<p align='center'>版权所有 &copy; 2026 MarkEdit 贡献者。<br>"
        "采用 MIT 许可证授权。</p>")
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
