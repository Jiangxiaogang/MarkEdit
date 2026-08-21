#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFont>
#include <QList>
#include <QString>
#include <QSettings>

class ConfigManager : public QObject
{
    Q_OBJECT
    
public:
    static ConfigManager* instance();
    
    void loadConfig();
    void saveConfig();
    
    // 编辑器设置
    QFont editorFont() const;
    void setEditorFont(const QFont &font);
    
    bool showLineNumbers() const;
    void setShowLineNumbers(bool show);
    
    bool showWhitespace() const;
    void setShowWhitespace(bool show);
    
    int tabWidth() const;
    void setTabWidth(int width);
    
    // 预览设置
    QString cssFilePath() const;
    void setCssFilePath(const QString &path);
    
    bool autoRefresh() const;
    void setAutoRefresh(bool refresh);
    
    bool syncScroll() const;
    void setSyncScroll(bool sync);
    
    // 窗口设置
    QList<int> splitterSizes() const;
    void setSplitterSizes(const QList<int> &sizes);
    
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray &geometry);
    
    QByteArray windowState() const;
    void setWindowState(const QByteArray &state);
    
    // 默认 CSS
    static QString defaultCSS();
    
private:
    explicit ConfigManager(QObject *parent = 0);
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    void ensureDefaults();
    
private:
    QSettings *m_settings;
    
    // 编辑器设置
    QFont m_editorFont;
    bool m_showLineNumbers;
    bool m_showWhitespace;
    int m_tabWidth;
    
    // 预览设置
    QString m_cssFilePath;
    bool m_autoRefresh;
    bool m_syncScroll;
    
    // 窗口设置
    QList<int> m_splitterSizes;
    QByteArray m_windowGeometry;
    QByteArray m_windowState;
};

#endif // CONFIGMANAGER_H
