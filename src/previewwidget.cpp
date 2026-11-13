#include "previewwidget.h"
#include "markdownparser.h"
#include "stylesheetloader.h"
#include "configmanager.h"

#include <QUrl>
#include <QTimer>
#include <QKeyEvent>
#include <QKeySequence>
#include <QDesktopServices>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSettings>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWebView(parent)
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
    applyStyle();
    applyFontSettings();
    connect(ConfigManager::instance(), SIGNAL(configurationChanged()), this, SLOT(onConfigChanged()));
}

PreviewWidget::~PreviewWidget()
{
}

QString PreviewWidget::getHtml() const
{
    return page()->mainFrame()->toHtml();
}

void PreviewWidget::setMarkdown(const QString &markdown)
{
    m_markdown = markdown;
    refresh();
}

void PreviewWidget::refresh()
{
    // setHtml() reloads the document and resets the scroll bar to the top.
    // Remember the current relative position so re-rendering (for example
    // while typing) keeps the preview where the user left it, regardless of
    // whether editor/preview scroll sync is enabled.
    const int max = scrollMaximum();
    const float ratio = max > 0 ? static_cast<float>(scrollValue()) / max : 0.0f;

    QString body = MarkdownParser::parse(m_markdown);
    QString html = generateHtml(body);
    m_emitScroll = false;
    m_lastScroll = 0;
    m_pendingRatio = ratio;
    setHtml(html, m_baseUrl);
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
    settings->setFontFamily(QWebSettings::FixedFont, cfg->monospaceFont());
    settings->setFontSize(QWebSettings::DefaultFontSize, cfg->standardFontSize());
    settings->setFontSize(QWebSettings::DefaultFixedFontSize, cfg->monospaceFontSize());
    refresh();
}

void PreviewWidget::applyStyle()
{
    ConfigManager *cfg = ConfigManager::instance();
    // An empty configured path selects the built-in style.
    m_css = StyleSheetLoader::load(cfg->previewStyleFile());
}

void PreviewWidget::onConfigChanged()
{
    applyStyle();
    applyFontSettings();
}

QString PreviewWidget::generateHtml(const QString &body) const
{
    return QString(
                "<!DOCTYPE html><html><head><meta charset=\"utf-8\">\n"
                "<style>%1</style></head>\n"
                "<body>%2</body></html>")
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

void PreviewWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Copy))
    {
        triggerPageAction(QWebPage::Copy);
        event->accept();
        return;
    }
    if (event->matches(QKeySequence::SelectAll))
    {
        triggerPageAction(QWebPage::SelectAll);
        event->accept();
        return;
    }
    QWebView::keyPressEvent(event);
}

void PreviewWidget::onLinkClicked(const QUrl &url)
{
    if (url.scheme().startsWith("http") || url.scheme().startsWith("file"))
    {
        QDesktopServices::openUrl(url);
    }
}
