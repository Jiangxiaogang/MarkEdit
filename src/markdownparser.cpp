#include "markdownparser.h"
#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"
#include <QRegExp>
#include <QStringList>

MarkdownParser::MarkdownParser(QObject *parent)
    : QObject(parent)
{
}

QString MarkdownParser::parse(const QString &markdown) const
{
    QByteArray utf8Data = markdown.toUtf8();
    const char *text = utf8Data.constData();
    size_t len = static_cast<size_t>(utf8Data.size());

    cmark_gfm_core_extensions_ensure_registered();
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    cmark_syntax_extension *table_ext = cmark_find_syntax_extension("table");
    if (table_ext)
    {
        cmark_parser_attach_syntax_extension(parser, table_ext);
    }
    cmark_parser_feed(parser, text, len);
    cmark_node *document = cmark_parser_finish(parser);
    int options = CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES | CMARK_OPT_GITHUB_PRE_LANG | CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES | CMARK_OPT_FOOTNOTES;
    char *html_output = cmark_render_html(document, options, NULL);
    QString html = QString::fromUtf8(html_output);
    free(html_output);
    cmark_node_free(document);
    cmark_parser_free(parser);

    return html;
}
