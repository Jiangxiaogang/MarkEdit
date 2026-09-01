#include "settingsdialog.h"
#include "configmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QLabel>
#include <QFont>
#include <QColorDialog>

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
    m_fontSize->setRange(6, 99);
    m_tabWidth = new QSpinBox;
    m_tabWidth->setRange(2, 16);

    QFont cur = m_config->editorFont();
    m_fontCombo->setCurrentFont(cur);
    m_fontSize->setValue(cur.pointSize());
    m_tabWidth->setValue(m_config->tabWidth());

    m_currentLineColor = m_config->currentLineColor();
    m_currentLineEdit = new QLineEdit(m_currentLineColor.name());
    m_currentLineBtn = new QPushButton;
    m_currentLineBtn->setFixedSize(24, 24);
    setButtonColor(m_currentLineBtn, m_currentLineColor);

    QHBoxLayout *currentLineRow = new QHBoxLayout;
    currentLineRow->addWidget(m_currentLineEdit);
    currentLineRow->addWidget(m_currentLineBtn);

    QFormLayout *editorLayout = new QFormLayout(editorTab);
    editorLayout->addRow(tr("字体:"), m_fontCombo);
    editorLayout->addRow(tr("字号:"), m_fontSize);
    editorLayout->addRow(tr("制表符宽度:"), m_tabWidth);
    editorLayout->addRow(tr("当前行底色:"), currentLineRow);

    tabs->addTab(editorTab, tr("编辑器"));

    connect(m_currentLineBtn, SIGNAL(clicked()), this, SLOT(chooseCurrentLineColor()));
    connect(m_currentLineEdit, SIGNAL(editingFinished()), this, SLOT(applyCurrentLineText()));

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

void SettingsDialog::setButtonColor(QPushButton *btn, const QColor &color)
{
    btn->setAutoFillBackground(true);
    btn->setStyleSheet(QString("background-color: %1; min-height: 20px;").arg(color.name()));
}

void SettingsDialog::setColor(QColor &target, const QColor &color, QPushButton *btn, QLineEdit *edit)
{
    target = color;
    setButtonColor(btn, color);
    edit->setText(color.name());
}

void SettingsDialog::chooseCurrentLineColor()
{
    QColor c = QColorDialog::getColor(m_currentLineColor, this, tr("选择当前行背景颜色"));
    if (c.isValid())
        setColor(m_currentLineColor, c, m_currentLineBtn, m_currentLineEdit);
}

void SettingsDialog::applyCurrentLineText()
{
    QColor c(m_currentLineEdit->text());
    if (c.isValid())
        setColor(m_currentLineColor, c, m_currentLineBtn, m_currentLineEdit);
    else
        m_currentLineEdit->setText(m_currentLineColor.name());
}

void SettingsDialog::accept()
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_fontSize->value());
    m_config->setEditorFont(f);
    m_config->setTabWidth(m_tabWidth->value());
    m_config->setCurrentLineColor(m_currentLineColor.name());
    m_config->setStandardFont(m_standardFontCombo->currentFont().family());
    m_config->setSerifFont(m_serifFontCombo->currentFont().family());
    m_config->setSansSerifFont(m_sansSerifFontCombo->currentFont().family());
    m_config->setMonospaceFont(m_monospaceFontCombo->currentFont().family());
    m_config->saveConfig();
    QDialog::accept();
}
