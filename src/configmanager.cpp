#include "configmanager.h"
#include "markdownparser.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>

ConfigManager *ConfigManager::m_instance = 0;

namespace
{
    const QString &configFilePath()
    {
        QString exeDir = QCoreApplication::applicationDirPath();
        static QString configPath = QDir(exeDir).filePath("config.ini");
        return configPath;
    }
} // namespace

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
    QSettings settings(configFilePath(), QSettings::IniFormat);

    // Editor
    QVariant fontVar = settings.value("editor/font", "Simsun,12,-1,5,50,0,0,0,0,0");
    m_editorFont.fromString(fontVar.toString());
    m_lineWrap = settings.value("editor/line_wrap", false).toBool();
    m_showLineNumber = settings.value("editor/show_line_number", true).toBool();
    m_showWhitespace = settings.value("editor/show_whitespace", false).toBool();
    m_showSyntaxHighlighting = settings.value("editor/syntax_highlighting", true).toBool();
    m_tabWidth = settings.value("editor/tab_width", 4).toInt();
    m_defaultEncoding = settings.value("editor/default_encoding", "UTF-8").toString();
    m_currentLineColor = settings.value("editor/current_line_color", "#f0f0ff").toString();
    m_headingColor = settings.value("editor/heading_color", "#0000ff").toString();
    m_codeColor    = settings.value("editor/code_color", "#008000").toString();
    m_quoteColor   = settings.value("editor/quote_color", "#ff0000").toString();
    m_listColor    = settings.value("editor/list_color", "#ff0000").toString();
    m_hrColor      = settings.value("editor/hr_color", "#000000").toString();
    m_boldColor    = settings.value("editor/bold_color", "#800000").toString();
    m_italicColor  = settings.value("editor/italic_color", "#808000").toString();
    m_strikeColor  = settings.value("editor/strike_color", "#808080").toString();
    m_linkColor    = settings.value("editor/link_color", "#e06010").toString();
    m_tableColor   = settings.value("editor/table_color", "#000000").toString();

    // Parser render options
    m_parserOptions.clear();
    foreach (const ParserOption &o, MarkdownParser::parserOptions())
        m_parserOptions.insert(o.key, settings.value("parser/" + o.key, o.defaultOn).toBool());

    // Preview (browser font families)
    m_standardFont   = settings.value("preview/standard_font", "SimSun").toString();
    m_serifFont      = settings.value("preview/serif_font", "SimSun").toString();
    m_sansSerifFont  = settings.value("preview/sans_serif_font", "Tahoma").toString();
    m_monospaceFont  = settings.value("preview/monospace_font", "Courier New").toString();
    m_previewStyleFile = settings.value("preview/style_file", "").toString();

    // Window
    m_windowGeometry = settings.value("window/geometry").toByteArray();
    m_windowState = settings.value("window/state").toByteArray();

    // Recent files
    m_recentFiles = settings.value("recent/files").toStringList();
}

void ConfigManager::saveConfig()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);

    settings.setValue("editor/font", m_editorFont.toString());
    settings.setValue("editor/line_wrap", m_lineWrap);
    settings.setValue("editor/show_line_number", m_showLineNumber);
    settings.setValue("editor/show_whitespace", m_showWhitespace);
    settings.setValue("editor/syntax_highlighting", m_showSyntaxHighlighting);
    settings.setValue("editor/tab_width", m_tabWidth);
    settings.setValue("editor/default_encoding", m_defaultEncoding);
    settings.setValue("editor/current_line_color", m_currentLineColor);
    settings.setValue("editor/heading_color", m_headingColor);
    settings.setValue("editor/code_color", m_codeColor);
    settings.setValue("editor/quote_color", m_quoteColor);
    settings.setValue("editor/list_color", m_listColor);
    settings.setValue("editor/hr_color", m_hrColor);
    settings.setValue("editor/bold_color", m_boldColor);
    settings.setValue("editor/italic_color", m_italicColor);
    settings.setValue("editor/strike_color", m_strikeColor);
    settings.setValue("editor/link_color", m_linkColor);
    settings.setValue("editor/table_color", m_tableColor);

    // Parser render options
    foreach (const ParserOption &o, MarkdownParser::parserOptions())
        settings.setValue("parser/" + o.key, m_parserOptions.value(o.key));

    settings.setValue("preview/standard_font", m_standardFont);
    settings.setValue("preview/serif_font", m_serifFont);
    settings.setValue("preview/sans_serif_font", m_sansSerifFont);
    settings.setValue("preview/monospace_font", m_monospaceFont);
    settings.setValue("preview/style_file", m_previewStyleFile);

    settings.setValue("window/geometry", m_windowGeometry);
    settings.setValue("window/state", m_windowState);

    settings.setValue("recent/files", m_recentFiles);

    emit configurationChanged();
}

// ---- Editor settings ----
QFont ConfigManager::editorFont() const
{
    return m_editorFont;
}
void ConfigManager::setEditorFont(const QFont &font)
{
    m_editorFont = font;
}

bool ConfigManager::lineWrap() const
{
    return m_lineWrap;
}
void ConfigManager::setLineWrap(bool enabled)
{
    m_lineWrap = enabled;
}

bool ConfigManager::showLineNumber() const
{
    return m_showLineNumber;
}
void ConfigManager::setShowLineNumber(bool show)
{
    m_showLineNumber = show;
}

bool ConfigManager::showWhitespace() const
{
    return m_showWhitespace;
}
void ConfigManager::setShowWhitespace(bool show)
{
    m_showWhitespace = show;
}

bool ConfigManager::showSyntaxHighlighting() const
{
    return m_showSyntaxHighlighting;
}
void ConfigManager::setShowSyntaxHighlighting(bool show)
{
    m_showSyntaxHighlighting = show;
}

int ConfigManager::tabWidth() const
{
    return m_tabWidth;
}
void ConfigManager::setTabWidth(int width)
{
    m_tabWidth = width;
}

// ---- Editor colours ----
QString ConfigManager::currentLineColor() const { return m_currentLineColor; }
void ConfigManager::setCurrentLineColor(const QString &color) { m_currentLineColor = color; }

QString ConfigManager::headingColor() const { return m_headingColor; }
void ConfigManager::setHeadingColor(const QString &color) { m_headingColor = color; }

QString ConfigManager::codeColor() const { return m_codeColor; }
void ConfigManager::setCodeColor(const QString &color) { m_codeColor = color; }

QString ConfigManager::quoteColor() const { return m_quoteColor; }
void ConfigManager::setQuoteColor(const QString &color) { m_quoteColor = color; }

QString ConfigManager::listColor() const { return m_listColor; }
void ConfigManager::setListColor(const QString &color) { m_listColor = color; }

QString ConfigManager::hrColor() const { return m_hrColor; }
void ConfigManager::setHrColor(const QString &color) { m_hrColor = color; }

QString ConfigManager::boldColor() const { return m_boldColor; }
void ConfigManager::setBoldColor(const QString &color) { m_boldColor = color; }

QString ConfigManager::italicColor() const { return m_italicColor; }
void ConfigManager::setItalicColor(const QString &color) { m_italicColor = color; }

QString ConfigManager::strikeColor() const { return m_strikeColor; }
void ConfigManager::setStrikeColor(const QString &color) { m_strikeColor = color; }

QString ConfigManager::linkColor() const { return m_linkColor; }
void ConfigManager::setLinkColor(const QString &color) { m_linkColor = color; }

QString ConfigManager::tableColor() const { return m_tableColor; }
void ConfigManager::setTableColor(const QString &color) { m_tableColor = color; }

// ---- Encoding ----
QString ConfigManager::defaultEncoding() const
{
    return m_defaultEncoding;
}
void ConfigManager::setDefaultEncoding(const QString &encoding)
{
    m_defaultEncoding = encoding;
}

// ---- Parser render options ----
bool ConfigManager::parserOption(const QString &key) const
{
    return m_parserOptions.value(key, false);
}

void ConfigManager::setParserOption(const QString &key, bool on)
{
    m_parserOptions.insert(key, on);
}

// ---- Preview settings (browser font families) ----
QString ConfigManager::standardFont() const { return m_standardFont; }
void ConfigManager::setStandardFont(const QString &family) { m_standardFont = family; }

QString ConfigManager::serifFont() const { return m_serifFont; }
void ConfigManager::setSerifFont(const QString &family) { m_serifFont = family; }

QString ConfigManager::sansSerifFont() const { return m_sansSerifFont; }
void ConfigManager::setSansSerifFont(const QString &family) { m_sansSerifFont = family; }

QString ConfigManager::monospaceFont() const { return m_monospaceFont; }
void ConfigManager::setMonospaceFont(const QString &family) { m_monospaceFont = family; }

QString ConfigManager::previewStyleFile() const { return m_previewStyleFile; }
void ConfigManager::setPreviewStyleFile(const QString &path) { m_previewStyleFile = path; }

// ---- Window state ----
QByteArray ConfigManager::windowGeometry() const
{
    return m_windowGeometry;
}
void ConfigManager::setWindowGeometry(const QByteArray &geometry)
{
    m_windowGeometry = geometry;
}

QByteArray ConfigManager::windowState() const
{
    return m_windowState;
}
void ConfigManager::setWindowState(const QByteArray &state)
{
    m_windowState = state;
}

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
