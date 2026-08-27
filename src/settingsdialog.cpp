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
    setWindowTitle(tr("Preferences"));
    setModal(true);

    QTabWidget *tabs = new QTabWidget(this);

    // ---- Editor tab ----
    QWidget *editorTab = new QWidget;
    m_fontCombo = new QFontComboBox;
    m_fontSize = new QSpinBox;
    m_fontSize->setRange(6, 72);
    m_tabWidth = new QSpinBox;
    m_tabWidth->setRange(2, 8);
    m_lineNumbers = new QCheckBox(tr("Show line numbers"));
    m_whitespace = new QCheckBox(tr("Show whitespace characters"));
    m_syntaxHighlight = new QCheckBox(tr("Syntax highlighting"));

    QFont cur = m_config->editorFont();
    m_fontCombo->setCurrentFont(cur);
    m_fontSize->setValue(cur.pointSize());
    m_tabWidth->setValue(m_config->tabWidth());
    m_lineNumbers->setChecked(m_config->showLineNumbers());
    m_whitespace->setChecked(m_config->showWhitespace());
    m_syntaxHighlight->setChecked(m_config->showSyntaxHighlighting());

    QFormLayout *editorLayout = new QFormLayout(editorTab);
    editorLayout->addRow(tr("Font:"), m_fontCombo);
    editorLayout->addRow(tr("Font size:"), m_fontSize);
    editorLayout->addRow(tr("Tab width:"), m_tabWidth);
    editorLayout->addRow(m_lineNumbers);
    editorLayout->addRow(m_whitespace);
    editorLayout->addRow(m_syntaxHighlight);
    tabs->addTab(editorTab, tr("Editor"));

    // ---- Preview tab ----
    QWidget *previewTab = new QWidget;
    m_cssPath = new QLineEdit;
    m_cssPath->setText(m_config->cssFilePath());
    QPushButton *browseBtn = new QPushButton(tr("Browse..."));
    m_syncScroll = new QCheckBox(tr("Synchronise scroll with editor"));
    m_autoRefresh = new QCheckBox(tr("Auto refresh preview"));

    QHBoxLayout *cssRow = new QHBoxLayout;
    cssRow->addWidget(m_cssPath);
    cssRow->addWidget(browseBtn);

    QFormLayout *previewLayout = new QFormLayout(previewTab);
    previewLayout->addRow(tr("CSS file:"), cssRow);
    previewLayout->addRow(m_syncScroll);
    previewLayout->addRow(m_autoRefresh);
    tabs->addTab(previewTab, tr("Preview"));

    QPushButton *okBtn = new QPushButton(tr("OK"));
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(okBtn);
    btnRow->addWidget(cancelBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(tabs);
    main->addLayout(btnRow);

    connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::chooseCssFile);
    connect(okBtn, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::chooseCssFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select CSS file"),
                                                m_cssPath->text(),
                                                tr("CSS files (*.css);;All files (*)"));
    if (!path.isEmpty())
        m_cssPath->setText(path);
}

void SettingsDialog::accept()
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_fontSize->value());
    m_config->setEditorFont(f);
    m_config->setShowLineNumbers(m_lineNumbers->isChecked());
    m_config->setShowWhitespace(m_whitespace->isChecked());
    m_config->setShowSyntaxHighlighting(m_syntaxHighlight->isChecked());
    m_config->setTabWidth(m_tabWidth->value());
    m_config->setCssFilePath(m_cssPath->text());
    m_config->setSyncScroll(m_syncScroll->isChecked());
    m_config->setAutoRefresh(m_autoRefresh->isChecked());
    m_config->saveConfig();
    QDialog::accept();
}
