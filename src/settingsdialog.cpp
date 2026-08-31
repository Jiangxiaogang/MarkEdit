#include "settingsdialog.h"
#include "configmanager.h"

#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLabel>
#include <QFileDialog>
#include <QFont>

SettingsDialog::SettingsDialog(ConfigManager *config, QWidget *parent)
    : QDialog(parent)
    , m_config(config)
{
    setWindowTitle(tr("首选项"));
    setModal(true);

    QTabWidget *tabs = new QTabWidget(this);

    // ---- Editor tab ----
    QWidget *editorTab = new QWidget;
    m_fontCombo = new QFontComboBox;
    m_fontSize = new QSpinBox;
    m_fontSize->setRange(6, 72);
    m_tabWidth = new QSpinBox;
    m_tabWidth->setRange(2, 8);

    QFont cur = m_config->editorFont();
    m_fontCombo->setCurrentFont(cur);
    m_fontSize->setValue(cur.pointSize());
    m_tabWidth->setValue(m_config->tabWidth());

    QFormLayout *editorLayout = new QFormLayout(editorTab);
    editorLayout->addRow(tr("字体:"), m_fontCombo);
    editorLayout->addRow(tr("字号:"), m_fontSize);
    editorLayout->addRow(tr("制表符宽度:"), m_tabWidth);
    tabs->addTab(editorTab, tr("编辑器"));

    // ---- Preview tab ----
    QWidget *previewTab = new QWidget;
    m_cssPath = new QLineEdit;
    m_cssPath->setText(m_config->cssFilePath());
    QPushButton *browseBtn = new QPushButton(tr("浏览..."));

    QHBoxLayout *cssRow = new QHBoxLayout;
    cssRow->addWidget(m_cssPath);
    cssRow->addWidget(browseBtn);

    QFormLayout *previewLayout = new QFormLayout(previewTab);
    previewLayout->addRow(tr("CSS 文件:"), cssRow);
    tabs->addTab(previewTab, tr("预览"));

    QPushButton *okBtn = new QPushButton(tr("确定"));
    QPushButton *cancelBtn = new QPushButton(tr("取消"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(okBtn);
    btnRow->addWidget(cancelBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(tabs);
    main->addLayout(btnRow);

    connect(browseBtn, SIGNAL(clicked()), this, SLOT(chooseCssFile()));
    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

void SettingsDialog::chooseCssFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("选择 CSS 文件"),
                   m_cssPath->text(),
                   tr("CSS 文件 (*.css);;所有文件 (*)"));
    if (!path.isEmpty())
        m_cssPath->setText(path);
}

void SettingsDialog::accept()
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_fontSize->value());
    m_config->setEditorFont(f);
    m_config->setTabWidth(m_tabWidth->value());
    m_config->setCssFilePath(m_cssPath->text());
    m_config->saveConfig();
    QDialog::accept();
}
