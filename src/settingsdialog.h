#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QFontComboBox>
#include <QPushButton>
#include <QComboBox>

class ConfigManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigManager *config, QWidget *parent = 0);

private slots:
    void accept();
    void chooseCurrentLineColor();
    void applyCurrentLineText();
    void applyChanges();

private:
    ConfigManager *m_config;
    QFontComboBox *m_fontCombo;
    QSpinBox *m_fontSize;
    QSpinBox *m_tabWidth;
    QFontComboBox *m_standardFontCombo;
    QSpinBox *m_standardFontSize;
    QFontComboBox *m_monospaceFontCombo;
    QSpinBox *m_monospaceFontSize;
    QComboBox *m_styleCombo;
    QPushButton *m_currentLineBtn;
    QLineEdit *m_currentLineEdit;
    QColor m_currentLineColor;

    void setButtonColor(QPushButton *btn, const QColor &color);
    void setColor(QColor &target, const QColor &color, QPushButton *btn, QLineEdit *edit);
};

#endif // SETTINGSDIALOG_H
