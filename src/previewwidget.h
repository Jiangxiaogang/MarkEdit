#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWebView>
#include <QString>

class ConfigManager;

class PreviewWidget : public QWebView
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();
    QString getHtml() const;
    void setMarkdown(const QString &markdown);
    void setBaseUrl(const QUrl &url);
    void refresh();
    int scrollMaximum() const;
    int scrollValue() const;
    void setScrollRatio(float ratio);

signals:
    void scrolled(int value);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void onLoadFinished(bool ok);
    void onScrollTimeout();
    void onLinkClicked(const QUrl &url);
    void applyFontSettings();
    void applyStyle();
    void onConfigChanged();

private:
    QString generateHtml(const QString &body) const;
    void applyRatio(float ratio);

    QString m_markdown;
    QString m_css;
    QUrl m_baseUrl;
    bool m_emitScroll;
    QTimer *m_scrollTimer;
    int m_lastScroll;
    float m_pendingRatio;
};

#endif // PREVIEWWIDGET_H
