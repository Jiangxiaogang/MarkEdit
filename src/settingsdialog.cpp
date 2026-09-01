#include "settingsdialog.h"
#include "configmanager.h"

#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLabel>
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
    m_standardFontCombo = new QFontComboBox;
    m_serifFontCombo = new QFontComboBox;
    m_sansSerifFontCombo = new QFontComboBox;
    m_monospaceFontCombo = new QFontComboBox;

    m_standardFontCombo->setCurrentFont(QFont(m_config->standardFont()));
    m_serifFontCombo->setCurrentFont(QFont(m_config->serifFont()));
    m_sansSerifFontCombo->setCurrentFont(QFont(m_config->sansSerifFont()));
    m_monospaceFontCombo->setCurrentFont(QFont(m_config->monospaceFont()));

    QFormLayout *previewLayout = new QFormLayout(previewTab);
    previewLayout->addRow(tr("标准字体:"), m_standardFontCombo);
    previewLayout->addRow(tr("衬线字体:"), m_serifFontCombo);
    previewLayout->addRow(tr("无衬线字体:"), m_sansSerifFontCombo);
    previewLayout->addRow(tr("等宽字体:"), m_monospaceFontCombo);
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

    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

void SettingsDialog::accept()
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_fontSize->value());
    m_config->setEditorFont(f);
    m_config->setTabWidth(m_tabWidth->value());
    m_config->setStandardFont(m_standardFontCombo->currentFont().family());
    m_config->setSerifFont(m_serifFontCombo->currentFont().family());
    m_config->setSansSerifFont(m_sansSerifFontCombo->currentFont().family());
    m_config->setMonospaceFont(m_monospaceFontCombo->currentFont().family());
    m_config->saveConfig();
    QDialog::accept();
}
