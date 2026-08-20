#include "configmanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_showLineNumbers(true)
    , m_showWhitespace(false)
    , m_tabWidth(4)
    , m_autoRefresh(true)
    , m_syncScroll(true)
{
    // 配置文件路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_settings = new QSettings(configPath + "/config.ini", QSettings::IniFormat, this);
    
    ensureDefaults();
    loadConfig();
}

ConfigManager::~ConfigManager()
{
}

ConfigManager* ConfigManager::instance()
{
    static ConfigManager instance;
    return &instance;
}

void ConfigManager::ensureDefaults()
{
    // 设置默认字体
    m_editorFont = QFont("Consolas", 12);
    m_editorFont.setStyleHint(QFont::Monospace);
    
    // 默认 CSS
    m_cssFilePath = "";
}

void ConfigManager::loadConfig()
{
    m_settings->beginGroup("Editor");
    m_showLineNumbers = m_settings->value("showLineNumbers", true).toBool();
    m_showWhitespace = m_settings->value("showWhitespace", false).toBool();
    m_tabWidth = m_settings->value("tabWidth", 4).toInt();
    
    QString fontFamily = m_settings->value("fontFamily", "Consolas").toString();
    int fontSize = m_settings->value("fontSize", 12).toInt();
    m_editorFont = QFont(fontFamily, fontSize);
    m_editorFont.setStyleHint(QFont::Monospace);
    m_settings->endGroup();
    
    m_settings->beginGroup("Preview");
    m_cssFilePath = m_settings->value("cssFilePath", "").toString();
    m_autoRefresh = m_settings->value("autoRefresh", true).toBool();
    m_syncScroll = m_settings->value("syncScroll", true).toBool();
    m_settings->endGroup();
    
    m_settings->beginGroup("Window");
    // 使用 QVariant::fromValue() 来存储 QList<int>，然后正确转换回 QList<int>
    QVariant defaultSplitterSizes = QVariant::fromValue(QList<int>() << 400 << 400);
    QVariant splitterVar = m_settings->value("splitterSizes", defaultSplitterSizes);
    QList<QVariant> variantList = splitterVar.toList();
    QList<int> sizes;
    for (const QVariant &v : variantList) {
        sizes.append(v.toInt());
    }
    m_splitterSizes = sizes;
    m_windowGeometry = m_settings->value("geometry", QByteArray()).toByteArray();
    m_windowState = m_settings->value("state", QByteArray()).toByteArray();
    m_settings->endGroup();
}

void ConfigManager::saveConfig()
{
    m_settings->beginGroup("Editor");
    m_settings->setValue("showLineNumbers", m_showLineNumbers);
    m_settings->setValue("showWhitespace", m_showWhitespace);
    m_settings->setValue("tabWidth", m_tabWidth);
    m_settings->setValue("fontFamily", m_editorFont.family());
    m_settings->setValue("fontSize", m_editorFont.pointSize());
    m_settings->endGroup();
    
    m_settings->beginGroup("Preview");
    m_settings->setValue("cssFilePath", m_cssFilePath);
    m_settings->setValue("autoRefresh", m_autoRefresh);
    m_settings->setValue("syncScroll", m_syncScroll);
    m_settings->endGroup();
    
    m_settings->beginGroup("Window");
    // 使用 QVariant::fromValue() 来存储 QList<int>
    m_settings->setValue("splitterSizes", QVariant::fromValue(m_splitterSizes));
    m_settings->setValue("geometry", m_windowGeometry);
    m_settings->setValue("state", m_windowState);
    m_settings->endGroup();
    
    m_settings->sync();
}

QFont ConfigManager::editorFont() const
{
    return m_editorFont;
}

void ConfigManager::setEditorFont(const QFont &font)
{
    m_editorFont = font;
}

bool ConfigManager::showLineNumbers() const
{
    return m_showLineNumbers;
}

void ConfigManager::setShowLineNumbers(bool show)
{
    m_showLineNumbers = show;
}

bool ConfigManager::showWhitespace() const
{
    return m_showWhitespace;
}

void ConfigManager::setShowWhitespace(bool show)
{
    m_showWhitespace = show;
}

int ConfigManager::tabWidth() const
{
    return m_tabWidth;
}

void ConfigManager::setTabWidth(int width)
{
    m_tabWidth = width;
}

QString ConfigManager::cssFilePath() const
{
    return m_cssFilePath;
}

void ConfigManager::setCssFilePath(const QString &path)
{
    m_cssFilePath = path;
}

bool ConfigManager::autoRefresh() const
{
    return m_autoRefresh;
}

void ConfigManager::setAutoRefresh(bool refresh)
{
    m_autoRefresh = refresh;
}

bool ConfigManager::syncScroll() const
{
    return m_syncScroll;
}

void ConfigManager::setSyncScroll(bool sync)
{
    m_syncScroll = sync;
}

QList<int> ConfigManager::splitterSizes() const
{
    return m_splitterSizes;
}

void ConfigManager::setSplitterSizes(const QList<int> &sizes)
{
    m_splitterSizes = sizes;
}

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

QString ConfigManager::defaultCSS()
{
    // 尝试从 default.css 文件加载
    QString cssPath = QCoreApplication::applicationDirPath() + "/default.css";
    QFile file(cssPath);
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString css = in.readAll();
        file.close();
        return css;
    }
    
    // 如果文件不存在，返回空字符串或默认样式
    return QString();
}
