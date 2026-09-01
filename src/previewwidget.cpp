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
    , m_css(StyleSheetLoader::getDefaultCSS())
    , m_emitScroll(true)
    , m_lastScroll(0)
    , m_pendingRatio(-1.0f)
{
    setObjectName("previewWidget");
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    m_scrollTimer = new QTimer(this);
    m_scrollTimer->setInterval(100);
    connect(m_scrollTimer, SIGNAL(timeout()), this, SLOT(onScrollTimeout()));
    // Poll the scroll position so user scrolling in the preview is reported
    // via the `scrolled` signal and synced to the editor.
    m_scrollTimer->start();

    connect(this, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

    // Apply the configured browser font families to the preview.
    applyFontSettings();
    connect(ConfigManager::instance(), SIGNAL(configurationChanged()),
            this, SLOT(applyFontSettings()));
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
    // Suppress scroll reporting while the new content loads asynchronously,
    // otherwise the transient scroll position during reflow would be synced
    // back to the editor.
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
               "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
               "<style>%1</style></head><body>%2</body></html>")
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
    // setHtml() is asynchronous: re-apply the requested scroll position
    // once the new content has finished loading.
    if (m_pendingRatio >= 0.0f)
        applyRatio(m_pendingRatio);
    // Content has settled; resume reporting scroll changes to sync the editor.
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
    // Open external / local links in the default browser instead of navigating
    // inside the preview pane.
    if (url.scheme().startsWith("http") || url.scheme().startsWith("file"))
    {
        QDesktopServices::openUrl(url);
    }
}
