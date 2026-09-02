#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFont>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QColor>

class ConfigManager : public QObject
{
    Q_OBJECT
public:

    static ConfigManager *instance();

    void loadConfig();
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

    // Editor colours
    QString currentLineColor() const;
    void setCurrentLineColor(const QString &color);

    QString headingColor() const;
    void setHeadingColor(const QString &color);

    QString codeColor() const;
    void setCodeColor(const QString &color);

    QString quoteColor() const;
    void setQuoteColor(const QString &color);

    QString listColor() const;
    void setListColor(const QString &color);

    QString hrColor() const;
    void setHrColor(const QString &color);

    QString boldColor() const;
    void setBoldColor(const QString &color);

    QString italicColor() const;
    void setItalicColor(const QString &color);

    QString strikeColor() const;
    void setStrikeColor(const QString &color);

    QString linkColor() const;
    void setLinkColor(const QString &color);

    QString tableColor() const;
    void setTableColor(const QString &color);

    // ---- Encoding ----
    QString defaultEncoding() const;
    void setDefaultEncoding(const QString &encoding);

    // ---- Parser render options ----
    bool parserOption(const QString &key) const;
    void setParserOption(const QString &key, bool on);

    // ---- Preview settings (browser font families) ----
    QString standardFont() const;
    void setStandardFont(const QString &family);

    QString serifFont() const;
    void setSerifFont(const QString &family);

    QString sansSerifFont() const;
    void setSansSerifFont(const QString &family);

    QString monospaceFont() const;
    void setMonospaceFont(const QString &family);

    // Preview stylesheet file
    QString previewStyleFile() const;
    void setPreviewStyleFile(const QString &path);

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

    // Editor colours
    QString m_currentLineColor;
    QString m_headingColor;
    QString m_codeColor;
    QString m_quoteColor;
    QString m_listColor;
    QString m_hrColor;
    QString m_boldColor;
    QString m_italicColor;
    QString m_strikeColor;
    QString m_linkColor;
    QString m_tableColor;

    // Encoding
    QString m_defaultEncoding;

    // Parser render options
    QMap<QString, bool> m_parserOptions;

    // Preview
    QString m_standardFont;
    QString m_serifFont;
    QString m_sansSerifFont;
    QString m_monospaceFont;
    QString m_previewStyleFile;

    // Window
    QByteArray m_windowGeometry;
    QByteArray m_windowState;

    // Recent files
    QStringList m_recentFiles;
};

#endif // CONFIGMANAGER_H
