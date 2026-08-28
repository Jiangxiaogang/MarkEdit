#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWebView>
#include <QString>

class MarkdownParser;
class StyleSheetLoader;

/**
 * @brief Live Markdown preview widget.
 *
 * Renders parsed Markdown HTML with full CSS support using QtWebKit's
 * QWebView, which has far better CSS compatibility than QTextBrowser.
 * Supports proportional scroll synchronisation with the editor.
 */
class PreviewWidget : public QWebView
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();

    /**
     * @brief Set the CSS used to style the preview.
     */
    void setCSS(const QString &css);

    /**
     * @brief Set the raw Markdown source; triggers a re-render.
     */
    void setMarkdown(const QString &markdown);

    /**
     * @brief Force a re-render using the current Markdown + CSS.
     */
    void refresh();

    /**
     * @brief Maximum vertical scroll value of the rendered page.
     */
    int scrollMaximum() const;

    /**
     * @brief Current vertical scroll value of the rendered page.
     */
    int scrollValue() const;

    /**
     * @brief Proportionally scroll the preview (0.0 .. 1.0).
     */
    void setScrollRatio(float ratio);

signals:
    void scrolled(int value);
    void cssFailed(const QString &error);

private slots:
    void onLoadFinished(bool ok);
    void onScrollTimeout();
    void onLinkClicked(const QUrl &url);

private:
    QString generateHtml(const QString &body) const;
    void applyRatio(float ratio);

    MarkdownParser *m_parser;
    StyleSheetLoader *m_loader;
    QString m_markdown;
    QString m_css;
    bool m_emitScroll;
    QTimer *m_scrollTimer;
    int m_lastScroll;
    float m_pendingRatio;
};

#endif // PREVIEWWIDGET_H
