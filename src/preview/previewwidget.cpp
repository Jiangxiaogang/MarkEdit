#include "previewwidget.h"
#include "parser/markdownparser.h"
#include "config/configmanager.h"
#include <QFile>
#include <QTextStream>
#include <QWebPage>
#include <QDesktopServices>
#include <QUrl>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWebView(parent)
    , m_parser(new MarkdownParser(this))
    , m_cssLoader(nullptr)
{
    // 加载默认 CSS
    resetToDefaultCSS();
    
    // 连接 linkHovered 信号用于状态栏显示
    connect(page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)), this, SLOT(onLinkHovered(const QString&, const QString&, const QString&)));
    
    // 拦截链接点击来在外部浏览器打开 http/https 链接
    connect(page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(onLinkClicked(const QUrl&)));
}

PreviewWidget::~PreviewWidget()
{
}

void PreviewWidget::setMarkdownText(const QString &text)
{
    m_currentMarkdown = text;
    updatePreview(text);
}

void PreviewWidget::loadCSSFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        m_currentCSS = in.readAll();
        file.close();
        applyStyles();
    } else {
        // 加载失败时使用默认 CSS
        resetToDefaultCSS();
    }
}

void PreviewWidget::resetToDefaultCSS()
{
    m_currentCSS = ConfigManager::defaultCSS();
    applyStyles();
}

void PreviewWidget::updatePreview(const QString &markdown)
{
    m_currentMarkdown = markdown;
    QString html = m_parser->parse(markdown);
    QString styledHTML = generateHTML(html);
    setHtml(styledHTML);
}

void PreviewWidget::applyStyles()
{
    QString htmlContent = m_parser->parse(m_currentMarkdown);
    QString styledHTML = generateHTML(htmlContent);
    setHtml(styledHTML);
}

QString PreviewWidget::generateHTML(const QString &htmlContent)
{
    QString styledHTML = QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"utf-8\">"
        "<style>%1</style>"
        "</head>"
        "<body>%2</body>"
        "</html>"
    ).arg(m_currentCSS).arg(htmlContent);
    
    return styledHTML;
}

void PreviewWidget::onLinkHovered(const QString &link, const QString &title, const QString &textContent)
{
    m_linkHoveredUrl = link;
}

void PreviewWidget::onLinkClicked(const QUrl &url)
{
    if (url.scheme() == "http" || url.scheme() == "https") {
        QDesktopServices::openUrl(url);
    } else {
        // 其他链接正常导航
        QWebView::load(QNetworkRequest(url));
    }
}
