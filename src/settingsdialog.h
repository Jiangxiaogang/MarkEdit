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
    class QLineEdit *m_cssPath;
};

#endif // SETTINGSDIALOG_H
