#include "markdownparser.h"
#include "configmanager.h"
#include "cmark-gfm.h"
#include "cmark-gfm-core-extensions.h"

#include <QCoreApplication>
#include <QStringList>

QList<ParserOption> MarkdownParser::parserOptions()
{
    static const QList<ParserOption> options = QList<ParserOption>()
        << ParserOption{ "table",         QCoreApplication::translate("MarkdownParser", "表格"),          true,  0 }
        << ParserOption{ "strikethrough", QCoreApplication::translate("MarkdownParser", "删除线"),        true,  0 }
        << ParserOption{ "autolink",      QCoreApplication::translate("MarkdownParser", "自动链接"),      false, 0 }
        << ParserOption{ "tagfilter",     QCoreApplication::translate("MarkdownParser", "标签过滤"),      false, 0 }
        << ParserOption{ "tasklist",      QCoreApplication::translate("MarkdownParser", "任务列表"),      true,  0 }
        << ParserOption{ "hardbreaks",    QCoreApplication::translate("MarkdownParser", "保留单行换行"),  false, CMARK_OPT_HARDBREAKS };
    return options;
}

QString MarkdownParser::parse(const QString &markdown)
{
    QByteArray utf8Data = markdown.toUtf8();
    const char *text = utf8Data.constData();
    size_t len = static_cast<size_t>(utf8Data.size());

    cmark_gfm_core_extensions_ensure_registered();
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    ConfigManager *cfg = ConfigManager::instance();
    int renderOptions = CMARK_OPT_DEFAULT;
    foreach (const ParserOption &o, parserOptions())
    {
        if (cfg->parserOption(o.key))
        {
            renderOptions |= static_cast<int>(o.renderFlags);

            QByteArray nameBytes = o.key.toUtf8();
            cmark_syntax_extension *ext = cmark_find_syntax_extension(nameBytes.constData());
            if (ext)
            {
                cmark_parser_attach_syntax_extension(parser, ext);
            }
        }
    }

    cmark_parser_feed(parser, text, len);
    cmark_node *document = cmark_parser_finish(parser);
    char *html_output = cmark_render_html(document, renderOptions, NULL);
    QString html = QString::fromUtf8(html_output);
    free(html_output);
    cmark_node_free(document);
    cmark_parser_free(parser);
    return html;
}
