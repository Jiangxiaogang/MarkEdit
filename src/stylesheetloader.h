#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QObject>
#include <QString>

/**
 * @brief Loads and validates CSS for the preview pane.
 *
 * Provides a built-in default stylesheet (and a dark variant) so the preview
 * can always render even when no external CSS file is available.
 */
class StyleSheetLoader : public QObject
{
    Q_OBJECT
public:
    explicit StyleSheetLoader(QObject *parent = 0);

    /**
     * @brief Load CSS from @p filePath. Returns the CSS text on success,
     *        or the default stylesheet on failure.
     */
    QString loadFromFile(const QString &filePath);

    /**
     * @brief Built-in light theme stylesheet.
     */
    static QString getDefaultCSS();

    /**
     * @brief Built-in dark theme stylesheet.
     */
    static QString getDarkCSS();

    /**
     * @brief Minimal sanity validation (balanced braces).
     */
    bool validateCSS(const QString &css);

signals:
    void cssLoaded(const QString &css);
    void cssLoadFailed(const QString &error);
};

#endif // STYLESHEETLOADER_H
