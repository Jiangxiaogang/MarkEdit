#include "configmanager.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>

ConfigManager *ConfigManager::m_instance = nullptr;

ConfigManager *ConfigManager::instance()
{
    if (!m_instance)
        m_instance = new ConfigManager(QCoreApplication::instance());
    return m_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    loadConfig();
}

void ConfigManager::loadConfig()
{
    QSettings settings;

    // Editor
    QFont defFont("Consolas");
    defFont.setPointSize(12);
    QVariant fontVar = settings.value("editor/font");
    if (fontVar.isValid())
        m_editorFont.fromString(fontVar.toString());
    else
        m_editorFont = defFont;

    m_showLineNumbers = settings.value("editor/show_line_numbers", true).toBool();
    m_showWhitespace = settings.value("editor/show_whitespace", false).toBool();
    m_showSyntaxHighlighting = settings.value("editor/syntax_highlighting", true).toBool();
    m_tabWidth = settings.value("editor/tab_width", 4).toInt();

    // Preview
    m_cssFilePath = settings.value("preview/css_file_path", ":/styles/default.css").toString();
    m_syncScroll = settings.value("preview/sync_scroll", true).toBool();
    m_autoRefresh = settings.value("preview/auto_refresh", true).toBool();

    // Window
    m_windowGeometry = settings.value("window/geometry").toByteArray();
    m_windowState = settings.value("window/state").toByteArray();

    // Recent files
    m_recentFiles = settings.value("recent/files").toStringList();
}

void ConfigManager::saveConfig()
{
    QSettings settings;

    settings.setValue("editor/font", m_editorFont.toString());
    settings.setValue("editor/show_line_numbers", m_showLineNumbers);
    settings.setValue("editor/show_whitespace", m_showWhitespace);
    settings.setValue("editor/syntax_highlighting", m_showSyntaxHighlighting);
    settings.setValue("editor/tab_width", m_tabWidth);

    settings.setValue("preview/css_file_path", m_cssFilePath);
    settings.setValue("preview/sync_scroll", m_syncScroll);
    settings.setValue("preview/auto_refresh", m_autoRefresh);

    settings.setValue("window/geometry", m_windowGeometry);
    settings.setValue("window/state", m_windowState);

    settings.setValue("recent/files", m_recentFiles);

    emit configurationChanged();
}

// ---- Editor settings ----
QFont ConfigManager::editorFont() const { return m_editorFont; }
void ConfigManager::setEditorFont(const QFont &font) { m_editorFont = font; }

bool ConfigManager::showLineNumbers() const { return m_showLineNumbers; }
void ConfigManager::setShowLineNumbers(bool show) { m_showLineNumbers = show; }

bool ConfigManager::showWhitespace() const { return m_showWhitespace; }
void ConfigManager::setShowWhitespace(bool show) { m_showWhitespace = show; }

bool ConfigManager::showSyntaxHighlighting() const { return m_showSyntaxHighlighting; }
void ConfigManager::setShowSyntaxHighlighting(bool show) { m_showSyntaxHighlighting = show; }

int ConfigManager::tabWidth() const { return m_tabWidth; }
void ConfigManager::setTabWidth(int width) { m_tabWidth = width; }

// ---- Preview settings ----
QString ConfigManager::cssFilePath() const { return m_cssFilePath; }
void ConfigManager::setCssFilePath(const QString &path) { m_cssFilePath = path; }

bool ConfigManager::syncScroll() const { return m_syncScroll; }
void ConfigManager::setSyncScroll(bool sync) { m_syncScroll = sync; }

bool ConfigManager::autoRefresh() const { return m_autoRefresh; }
void ConfigManager::setAutoRefresh(bool refresh) { m_autoRefresh = refresh; }

// ---- Window state ----
QByteArray ConfigManager::windowGeometry() const { return m_windowGeometry; }
void ConfigManager::setWindowGeometry(const QByteArray &geometry) { m_windowGeometry = geometry; }

QByteArray ConfigManager::windowState() const { return m_windowState; }
void ConfigManager::setWindowState(const QByteArray &state) { m_windowState = state; }

// ---- Recent files ----
QStringList ConfigManager::recentFiles(int max) const
{
    QStringList list = m_recentFiles;
    while (list.size() > max)
        list.removeLast();
    return list;
}

void ConfigManager::addRecentFile(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists())
        return;
    QString absPath = info.absoluteFilePath();
    m_recentFiles.removeAll(absPath);
    m_recentFiles.prepend(absPath);
    while (m_recentFiles.size() > 10)
        m_recentFiles.removeLast();
}

void ConfigManager::clearRecentFiles()
{
    m_recentFiles.clear();
}
