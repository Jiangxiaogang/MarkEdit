#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWebView>
#include <QString>

class MarkdownParser;
class StyleSheetLoader;
class ConfigManager;

class PreviewWidget : public QWebView
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();

    void setCSS(const QString &css);
    void setMarkdown(const QString &markdown);
    void setBaseUrl(const QUrl &url);
    void refresh();
    int scrollMaximum() const;
    int scrollValue() const;
    void setScrollRatio(float ratio);

signals:
    void scrolled(int value);
    void cssFailed(const QString &error);

private slots:
    void onLoadFinished(bool ok);
    void onScrollTimeout();
    void onLinkClicked(const QUrl &url);
    void applyFontSettings();

private:
    QString generateHtml(const QString &body) const;
    void applyRatio(float ratio);

    MarkdownParser *m_parser;
    StyleSheetLoader *m_loader;
    QString m_markdown;
    QString m_css;
    QUrl m_baseUrl;
    bool m_emitScroll;
    QTimer *m_scrollTimer;
    int m_lastScroll;
    float m_pendingRatio;
};

#endif // PREVIEWWIDGET_H
