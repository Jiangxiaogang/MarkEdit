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
    void accept();

private:
    ConfigManager *m_config;
    class QFontComboBox *m_fontCombo;
    class QSpinBox *m_fontSize;
    class QSpinBox *m_tabWidth;
    class QFontComboBox *m_standardFontCombo;
    class QFontComboBox *m_serifFontCombo;
    class QFontComboBox *m_sansSerifFontCombo;
    class QFontComboBox *m_monospaceFontCombo;
};

#endif // SETTINGSDIALOG_H
