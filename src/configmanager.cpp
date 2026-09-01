#include "configmanager.h"
#include "markdownparser.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>

ConfigManager *ConfigManager::m_instance = 0;

namespace
{
    const QString &configFilePath()
    {
        static const QString path = "config.ini";
        return path;
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

    // Editor colours
    m_currentLineColor = settings.value("editor/current_line_color", QColor(60, 60, 60, 30)).value<QColor>();
    m_selectionColor   = settings.value("editor/selection_color", QColor(0, 120, 215)).value<QColor>();

    // Highlight colours (hex strings)
    m_headingColor = settings.value("highlighter/heading_color", "#1f6feb").toString();
    m_codeColor    = settings.value("highlighter/code_color", "#cf222e").toString();
    m_quoteColor   = settings.value("highlighter/quote_color", "#57606a").toString();
    m_listColor    = settings.value("highlighter/list_color", "#e36209").toString();
    m_hrColor      = settings.value("highlighter/hr_color", "#57606a").toString();
    m_boldColor    = settings.value("highlighter/bold_color", "#8250df").toString();
    m_italicColor  = settings.value("highlighter/italic_color", "#0c7b93").toString();
    m_strikeColor  = settings.value("highlighter/strike_color", "#57606a").toString();
    m_linkColor    = settings.value("highlighter/link_color", "#0550ae").toString();
    m_tableColor   = settings.value("highlighter/table_color", "#57606a").toString();

    // Parser render options
    m_parserOptions.clear();
    foreach (const ParserOption &o, MarkdownParser::parserOptions())
        m_parserOptions.insert(o.key, settings.value("parser/" + o.key, o.defaultOn).toBool());

    // Preview (browser font families)
    m_standardFont   = settings.value("preview/standard_font", "SimSun").toString();
    m_serifFont      = settings.value("preview/serif_font", "SimSun").toString();
    m_sansSerifFont  = settings.value("preview/sans_serif_font", "Microsoft YaHei").toString();
    m_monospaceFont  = settings.value("preview/monospace_font", "Courier New").toString();
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
    QSettings settings(configFilePath(), QSettings::IniFormat);

    settings.setValue("editor/font", m_editorFont.toString());
    settings.setValue("editor/line_wrap", m_lineWrap);
    settings.setValue("editor/show_line_number", m_showLineNumber);
    settings.setValue("editor/show_whitespace", m_showWhitespace);
    settings.setValue("editor/syntax_highlighting", m_showSyntaxHighlighting);
    settings.setValue("editor/tab_width", m_tabWidth);
    settings.setValue("editor/default_encoding", m_defaultEncoding);
    settings.setValue("editor/current_line_color", m_currentLineColor);
    settings.setValue("editor/selection_color", m_selectionColor);

    settings.setValue("highlighter/heading_color", m_headingColor);
    settings.setValue("highlighter/code_color", m_codeColor);
    settings.setValue("highlighter/quote_color", m_quoteColor);
    settings.setValue("highlighter/list_color", m_listColor);
    settings.setValue("highlighter/hr_color", m_hrColor);
    settings.setValue("highlighter/bold_color", m_boldColor);
    settings.setValue("highlighter/italic_color", m_italicColor);
    settings.setValue("highlighter/strike_color", m_strikeColor);
    settings.setValue("highlighter/link_color", m_linkColor);
    settings.setValue("highlighter/table_color", m_tableColor);

    // Parser render options
    foreach (const ParserOption &o, MarkdownParser::parserOptions())
        settings.setValue("parser/" + o.key, m_parserOptions.value(o.key));

    settings.setValue("preview/standard_font", m_standardFont);
    settings.setValue("preview/serif_font", m_serifFont);
    settings.setValue("preview/sans_serif_font", m_sansSerifFont);
    settings.setValue("preview/monospace_font", m_monospaceFont);
    settings.setValue("preview/sync_scroll", m_syncScroll);
    settings.setValue("preview/auto_refresh", m_autoRefresh);

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
QColor ConfigManager::currentLineColor() const { return m_currentLineColor; }
void ConfigManager::setCurrentLineColor(const QColor &color) { m_currentLineColor = color; }

QColor ConfigManager::selectionColor() const { return m_selectionColor; }
void ConfigManager::setSelectionColor(const QColor &color) { m_selectionColor = color; }

// ---- Encoding ----
QString ConfigManager::defaultEncoding() const
{
    return m_defaultEncoding;
}
void ConfigManager::setDefaultEncoding(const QString &encoding)
{
    m_defaultEncoding = encoding;
}

// ---- Highlight colours ----
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

bool ConfigManager::syncScroll() const
{
    return m_syncScroll;
}
void ConfigManager::setSyncScroll(bool sync)
{
    m_syncScroll = sync;
}

bool ConfigManager::autoRefresh() const
{
    return m_autoRefresh;
}
void ConfigManager::setAutoRefresh(bool refresh)
{
    m_autoRefresh = refresh;
}

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
