#include "previewwidget.h"
#include "markdownparser.h"
#include "stylesheetloader.h"
#include "configmanager.h"

#include <QUrl>
#include <QTimer>
#include <QDesktopServices>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSettings>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWebView(parent)
    , m_parser(new MarkdownParser(this))
    , m_loader(new StyleSheetLoader(this))
    , m_css("")
    , m_emitScroll(true)
    , m_lastScroll(0)
    , m_pendingRatio(-1.0f)
{
    setObjectName("previewWidget");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(100);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(onScrollTimeout()));
    m_scrollTimer->start();

    connect(this, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
    applyFontSettings();
    connect(ConfigManager::instance(), SIGNAL(configurationChanged()), this, SLOT(applyFontSettings()));
}

PreviewWidget::~PreviewWidget()
{
}

void PreviewWidget::setCSS(const QString &css)
{
    m_css = css;
    refresh();
}

void PreviewWidget::setMarkdown(const QString &markdown)
{
    m_markdown = markdown;
    refresh();
}

void PreviewWidget::refresh()
{
    QString body = m_parser->parse(m_markdown);
    QString html = generateHtml(body);
    m_emitScroll = false;
    setHtml(html, m_baseUrl);
    m_lastScroll = 0;
}

void PreviewWidget::setBaseUrl(const QUrl &url)
{
    m_baseUrl = url;
}

void PreviewWidget::applyFontSettings()
{
    QWebSettings *settings = QWebSettings::globalSettings();
    ConfigManager *cfg = ConfigManager::instance();
    settings->setFontFamily(QWebSettings::StandardFont, cfg->standardFont());
    settings->setFontFamily(QWebSettings::SerifFont, cfg->serifFont());
    settings->setFontFamily(QWebSettings::SansSerifFont, cfg->sansSerifFont());
    settings->setFontFamily(QWebSettings::FixedFont, cfg->monospaceFont());
    refresh();
}

QString PreviewWidget::generateHtml(const QString &body) const
{
    return QString(
               "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n"
               "<style>%1</style>\n</head>\n<body>\n%2\n</body>\n</html>")
           .arg(m_css)
           .arg(body);
}

int PreviewWidget::scrollMaximum() const
{
    QWebFrame *frame = page() ? page()->mainFrame() : 0;
    return frame ? frame->scrollBarMaximum(Qt::Vertical) : 0;
}

int PreviewWidget::scrollValue() const
{
    QWebFrame *frame = page() ? page()->mainFrame() : 0;
    return frame ? frame->scrollBarValue(Qt::Vertical) : 0;
}

void PreviewWidget::setScrollRatio(float ratio)
{
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    m_pendingRatio = ratio;
    applyRatio(ratio);
}

void PreviewWidget::applyRatio(float ratio)
{
    QWebFrame *frame = page() ? page()->mainFrame() : 0;
    if (!frame)
        return;
    int max = frame->scrollBarMaximum(Qt::Vertical);
    m_emitScroll = false;
    frame->setScrollBarValue(Qt::Vertical, static_cast<int>(max * ratio));
    m_lastScroll = static_cast<int>(max * ratio);
    m_emitScroll = true;
}

void PreviewWidget::onLoadFinished(bool ok)
{
    Q_UNUSED(ok);
    if (m_pendingRatio >= 0.0f)
        applyRatio(m_pendingRatio);
    m_emitScroll = true;
}

void PreviewWidget::onScrollTimeout()
{
    if (!m_emitScroll)
        return;
    int v = scrollValue();
    if (v != m_lastScroll)
    {
        m_lastScroll = v;
        emit scrolled(v);
    }
}

void PreviewWidget::onLinkClicked(const QUrl &url)
{
    if (url.scheme().startsWith("http") || url.scheme().startsWith("file"))
    {
        QDesktopServices::openUrl(url);
    }
}
