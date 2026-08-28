#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class ConfigManager;

/**
 * @brief Preferences dialog bound to ConfigManager.
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigManager *config, QWidget *parent = 0);

private slots:
    void chooseCssFile();
    void accept();

private:
    ConfigManager *m_config;
    class QFontComboBox *m_fontCombo;
    class QSpinBox *m_fontSize;
    class QSpinBox *m_tabWidth;
    class QCheckBox *m_lineNumbers;
    class QCheckBox *m_whitespace;
    class QCheckBox *m_syntaxHighlight;
    class QLineEdit *m_cssPath;
    class QCheckBox *m_syncScroll;
    class QCheckBox *m_autoRefresh;
};

#endif // SETTINGSDIALOG_H
