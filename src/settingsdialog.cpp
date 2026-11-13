#include "settingsdialog.h"
#include "configmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QFont>
#include <QColorDialog>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

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
    m_monospaceFontCombo = new QFontComboBox;
    m_standardFontSize = new QSpinBox;
    m_standardFontSize->setRange(8, 48);
    m_monospaceFontSize = new QSpinBox;
    m_monospaceFontSize->setRange(8, 48);

    m_standardFontCombo->setCurrentFont(QFont(m_config->standardFont()));
    m_monospaceFontCombo->setCurrentFont(QFont(m_config->monospaceFont()));
    m_standardFontSize->setValue(m_config->standardFontSize());
    m_monospaceFontSize->setValue(m_config->monospaceFontSize());

    m_styleCombo = new QComboBox;
    m_styleCombo->addItem(tr("内置样式"), QString()); // built-in: no external .css
    QDir stylesDir(QCoreApplication::applicationDirPath() + "/styles");
    if (stylesDir.exists())
    {
        QStringList files = stylesDir.entryList(QStringList() << "*.css", QDir::Files, QDir::Name);
        foreach (const QString &file, files)
            m_styleCombo->addItem(QFileInfo(file).baseName(), stylesDir.filePath(file));
    }
    int styleIdx = 0;
    QString curStyle = m_config->previewStyleFile();
    if (!curStyle.isEmpty())
    {
        int found = m_styleCombo->findData(curStyle);
        if (found >= 0)
            styleIdx = found;
    }
    m_styleCombo->setCurrentIndex(styleIdx);

    QFormLayout *previewLayout = new QFormLayout(previewTab);
    previewLayout->addRow(tr("样式文件:"), m_styleCombo);
    previewLayout->addRow(tr("标准字体:"), m_standardFontCombo);
    previewLayout->addRow(tr("标准字号:"), m_standardFontSize);
    previewLayout->addRow(tr("等宽字体:"), m_monospaceFontCombo);
    previewLayout->addRow(tr("等宽字号:"), m_monospaceFontSize);
    tabs->addTab(previewTab, tr("预览"));

    // Apply every change immediately. The dialog has no Cancel/rollback.
    connect(m_fontCombo, SIGNAL(currentFontChanged(QFont)), this, SLOT(applyChanges()));
    connect(m_fontSize, SIGNAL(valueChanged(int)), this, SLOT(applyChanges()));
    connect(m_tabWidth, SIGNAL(valueChanged(int)), this, SLOT(applyChanges()));
    connect(m_styleCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(applyChanges()));
    connect(m_standardFontCombo, SIGNAL(currentFontChanged(QFont)), this, SLOT(applyChanges()));
    connect(m_standardFontSize, SIGNAL(valueChanged(int)), this, SLOT(applyChanges()));
    connect(m_monospaceFontCombo, SIGNAL(currentFontChanged(QFont)), this, SLOT(applyChanges()));
    connect(m_monospaceFontSize, SIGNAL(valueChanged(int)), this, SLOT(applyChanges()));

    QPushButton *okBtn = new QPushButton(tr("确定"));
    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    btnRow->addWidget(okBtn);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addWidget(tabs);
    main->addLayout(btnRow);

    connect(okBtn, SIGNAL(clicked()), this, SLOT(accept()));
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
    {
        setColor(m_currentLineColor, c, m_currentLineBtn, m_currentLineEdit);
        applyChanges();
    }
}

void SettingsDialog::applyCurrentLineText()
{
    QColor c(m_currentLineEdit->text());
    if (!c.isValid())
    {
        m_currentLineEdit->setText(m_currentLineColor.name());
        return;
    }
    if (c == m_currentLineColor)
        return;

    setColor(m_currentLineColor, c, m_currentLineBtn, m_currentLineEdit);
    applyChanges();
}

void SettingsDialog::applyChanges()
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_fontSize->value());
    m_config->setEditorFont(f);
    m_config->setTabWidth(m_tabWidth->value());
    m_config->setCurrentLineColor(m_currentLineColor.name());
    m_config->setStandardFont(m_standardFontCombo->currentFont().family());
    m_config->setStandardFontSize(m_standardFontSize->value());
    m_config->setMonospaceFont(m_monospaceFontCombo->currentFont().family());
    m_config->setMonospaceFontSize(m_monospaceFontSize->value());
    m_config->setPreviewStyleFile(m_styleCombo->itemData(m_styleCombo->currentIndex()).toString());
    m_config->saveConfig();
    m_config->notifyChanged();
}

void SettingsDialog::accept()
{
    // Commit a colour typed into the line edit that may not have lost focus yet.
    applyCurrentLineText();
    QDialog::accept();
}
