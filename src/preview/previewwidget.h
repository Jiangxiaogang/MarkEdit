#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWebView>
#include <QString>
#include <QNetworkRequest>

class MarkdownParser;
class StyleSheetLoader;

class PreviewWidget : public QWebView
{
    Q_OBJECT
    
public:
    explicit PreviewWidget(QWidget *parent = 0);
    ~PreviewWidget();
    
    void setMarkdownText(const QString &text);
    void loadCSSFile(const QString &filePath);
    void resetToDefaultCSS();
    
public slots:
    void updatePreview(const QString &markdown);
    
private slots:
    void onLinkHovered(const QString &link, const QString &title, const QString &textContent);
    void onLinkClicked(const QUrl &url);
    
private:
    MarkdownParser *m_parser;
    StyleSheetLoader *m_cssLoader;
    QString m_currentCSS;
    QString m_currentMarkdown;
    QString m_linkHoveredUrl;
    
    void applyStyles();
    QString generateHTML(const QString &htmlContent);
};

#endif // PREVIEWWIDGET_H
