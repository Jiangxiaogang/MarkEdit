#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QTextBrowser>
#include <QString>

class MarkdownParser;
class StyleSheetLoader;

class PreviewWidget : public QTextBrowser
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
    
    void applyStyles();
    QString generateHTML(const QString &htmlContent);
};

#endif // PREVIEWWIDGET_H
