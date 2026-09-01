#include "aboutdialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSysInfo>
#include <qglobal.h>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("关于 MarkEdit"));
    setModal(true);

    QLabel *title = new QLabel(QString("<h3>MarkEdit</h3>"));
    title->setAlignment(Qt::AlignCenter);

    QLabel *version = new QLabel(QString("版本: V%1 (QT%2)").arg("1.0.0").arg(QT_VERSION_STR));
    version->setAlignment(Qt::AlignCenter);

    QString info = QString(
                       "<p>项目地址: <a href=https://www.github.com/jiangxiaogang/markedit>https://www.github.com/jiangxiaogang/markedit</a></p>"
                       "<p>采用 MIT 许可证授权。</p>");

    QLabel *details = new QLabel(info);
    details->setWordWrap(true);
    details->setOpenExternalLinks(true);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(box, SIGNAL(accepted()), this, SLOT(accept()));

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(title);
    main->addWidget(version);
    main->addWidget(details);
    main->addWidget(box);
}
