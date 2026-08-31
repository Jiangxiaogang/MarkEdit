#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFont>
#include <QString>
#include <QStringList>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Returns the global ConfigManager instance (lazy creation).
     */
    static ConfigManager *instance();

    /**
     * @brief Load configuration from disk (called once at startup).
     */
    void loadConfig();

    /**
     * @brief Persist the current configuration to disk.
     */
    void saveConfig();

    // ---- Editor settings ----
    QFont editorFont() const;
    void setEditorFont(const QFont &font);

    bool lineWrap() const;
    void setLineWrap(bool enabled);

    bool showLineNumber() const;
    void setShowLineNumber(bool show);

    bool showWhitespace() const;
    void setShowWhitespace(bool show);

    bool showSyntaxHighlighting() const;
    void setShowSyntaxHighlighting(bool show);

    int tabWidth() const;
    void setTabWidth(int width);

    // ---- Encoding ----
    // "Auto" means auto-detect on open; otherwise a codec name (e.g. "GB18030").
    QString defaultEncoding() const;
    void setDefaultEncoding(const QString &encoding);

    // ---- Preview settings ----
    QString cssFilePath() const;
    void setCssFilePath(const QString &path);

    bool syncScroll() const;
    void setSyncScroll(bool sync);

    bool autoRefresh() const;
    void setAutoRefresh(bool refresh);

    // ---- Window state ----
    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray &geometry);

    QByteArray windowState() const;
    void setWindowState(const QByteArray &state);

    // ---- Recent files ----
    QStringList recentFiles(int max = 10) const;
    void addRecentFile(const QString &path);
    void clearRecentFiles();

signals:
    void configurationChanged();

private:
    explicit ConfigManager(QObject *parent = 0);
    ConfigManager(const ConfigManager &) = delete;
    ConfigManager &operator=(const ConfigManager &) = delete;

    static ConfigManager *m_instance;

    // Editor
    QFont m_editorFont;
    bool m_lineWrap;
    bool m_showLineNumber;
    bool m_showWhitespace;
    bool m_showSyntaxHighlighting;
    int m_tabWidth;

    // Encoding
    QString m_defaultEncoding;

    // Preview
    QString m_cssFilePath;
    bool m_syncScroll;
    bool m_autoRefresh;

    // Window
    QByteArray m_windowGeometry;
    QByteArray m_windowState;

    // Recent files
    QStringList m_recentFiles;
};

#endif // CONFIGMANAGER_H
