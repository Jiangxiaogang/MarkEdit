#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWebEngineView>
#include <QString>

class MarkdownParser;
class StyleSheetLoader;

class PreviewWidget : public QWebEngineView
{
    Q_OBJECT
    
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();
    
    void setMarkdownText(const QString &text);
    void loadCSSFile(const QString &filePath);
    void resetToDefaultCSS();
    
public slots:
    void updatePreview(const QString &markdown);
    
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
