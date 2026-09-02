#include "markdownparser.h"
#include "configmanager.h"
#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"
#include <QStringList>

MarkdownParser::MarkdownParser(QObject *parent)
    : QObject(parent)
{
}

QList<ParserOption> MarkdownParser::parserOptions()
{
    static const QList<ParserOption> options = QList<ParserOption>()
        << ParserOption{ "table",         tr("表格"),        "table",         true  }
        << ParserOption{ "strikethrough", tr("删除线"),      "strikethrough", true }
        << ParserOption{ "autolink",      tr("自动链接"),    "autolink",      false }
        << ParserOption{ "tagfilter",     tr("标签过滤"),    "tagfilter",     false }
        << ParserOption{ "tasklist",      tr("任务列表"),    "tasklist",      true };
    return options;
}

QString MarkdownParser::parse(const QString &markdown) const
{
    QByteArray utf8Data = markdown.toUtf8();
    const char *text = utf8Data.constData();
    size_t len = static_cast<size_t>(utf8Data.size());

    cmark_gfm_core_extensions_ensure_registered();
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    ConfigManager *cfg = ConfigManager::instance();
    foreach (const ParserOption &o, parserOptions())
    {
        if (cfg->parserOption(o.key))
        {
            QByteArray nameBytes = o.name.toUtf8();
            cmark_syntax_extension *ext = cmark_find_syntax_extension(nameBytes.constData());
            if (ext)
            {
                cmark_parser_attach_syntax_extension(parser, ext);
            }
        }
    }

    cmark_parser_feed(parser, text, len);
    cmark_node *document = cmark_parser_finish(parser);
    char *html_output = cmark_render_html(document, CMARK_OPT_DEFAULT, NULL);
    QString html = QString::fromUtf8(html_output);
    free(html_output);
    cmark_node_free(document);
    cmark_parser_free(parser);
    return html;
}
