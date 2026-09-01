#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class ConfigManager;

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigManager *config, QWidget *parent = 0);

private slots:
    void accept();
    void chooseCurrentLineColor();
    void chooseSelectionColor();
    void applyCurrentLineText();
    void applySelectionText();

private:
    ConfigManager *m_config;
    class QFontComboBox *m_fontCombo;
    class QSpinBox *m_fontSize;
    class QSpinBox *m_tabWidth;
    class QFontComboBox *m_standardFontCombo;
    class QFontComboBox *m_serifFontCombo;
    class QFontComboBox *m_sansSerifFontCombo;
    class QFontComboBox *m_monospaceFontCombo;
    class QPushButton *m_currentLineBtn;
    class QPushButton *m_selectionBtn;
    class QLineEdit *m_currentLineEdit;
    class QLineEdit *m_selectionEdit;
    QColor m_currentLineColor;
    QColor m_selectionColor;

    void setButtonColor(QPushButton *btn, const QColor &color);
    void setColor(QColor &target, const QColor &color, QPushButton *btn, QLineEdit *edit);
};

#endif // SETTINGSDIALOG_H
