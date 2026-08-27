#include "../src/markdownparser.h"

#include <QCoreApplication>
#include <QString>
#include <cassert>
#include <iostream>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { std::cerr << "FAIL: " << msg << "\n"; ++failures; } \
    else { std::cout << "PASS: " << msg << "\n"; } \
} while (0)

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    MarkdownParser parser;

    // Headings
    QString h = parser.parse("# Title\n");
    CHECK(h.contains("<h1>Title</h1>"), "h1 heading");
    h = parser.parse("### Sub\n");
    CHECK(h.contains("<h3>Sub</h3>"), "h3 heading");

    // Bold / italic
    QString b = parser.parse("**bold** and *em*\n");
    CHECK(b.contains("<strong>bold</strong>"), "bold");
    CHECK(b.contains("<em>em</em>"), "italic");

    // Strikethrough
    CHECK(parser.parse("~~no~~\n").contains("<del>no</del>"), "strikethrough");

    // Intra-word underscores must NOT be treated as emphasis
    QString intra = parser.parse("AAA_BBB_CCC\n");
    CHECK(!intra.contains("<em>BBB</em>"), "intra-word underscore not italic");

    // Underscore emphasis at word boundaries still works
    QString uem = parser.parse("a _b_ c\n");
    CHECK(uem.contains("<em>b</em>"), "underscore italic at boundary");
    QString ubold = parser.parse("a __b__ c\n");
    CHECK(ubold.contains("<strong>b</strong>"), "underscore bold at boundary");

    // Inline code
    CHECK(parser.parse("use `code` now\n").contains("<code>code</code>"), "inline code");

    // Unordered list
    QString ul = parser.parse("- one\n- two\n");
    CHECK(ul.contains("<ul>") && ul.contains("<li>one</li>") && ul.contains("<li>two</li>"), "ul list");

    // Ordered list
    QString ol = parser.parse("1. first\n2. second\n");
    CHECK(ol.contains("<ol>") && ol.contains("<li>first</li>"), "ol list");

    // Block quote
    QString q = parser.parse("> quoted\n");
    CHECK(q.contains("<blockquote>") && q.contains("quoted"), "blockquote");

    // Code block
    QString cb = parser.parse("```\nint x = 1;\n```\n");
    CHECK(cb.contains("<pre><code>") && cb.contains("int x = 1;"), "fenced code block");

    // Link
    CHECK(parser.parse("[site](https://x.com)\n").contains("<a href=\"https://x.com\">site</a>"), "link");

    // Image
    CHECK(parser.parse("![alt](img.png)\n").contains("<img src=\"img.png\" alt=\"alt\">"), "image");

    // Horizontal rule
    CHECK(parser.parse("---\n").contains("<hr>"), "horizontal rule");

    // Table
    QString tbl = parser.parse("| A | B |\n| - | - |\n| 1 | 2 |\n");
    CHECK(tbl.contains("<table>") && tbl.contains("</table>"), "table render");
    CHECK(tbl.contains("<th>A</th>") && tbl.contains("<th>B</th>"), "table header");
    CHECK(tbl.contains("<td>1</td>") && tbl.contains("<td>2</td>"), "table cell");
    CHECK(tbl.contains("<thead>") && tbl.contains("<tbody>"), "table thead/tbody");

    // Table column alignment
    QString talign = parser.parse("| L | C | R |\n| :-- | :-: | --: |\n| a | b | c |\n");
    CHECK(talign.contains("text-align:left"), "table align left");
    CHECK(talign.contains("text-align:center"), "table align center");
    CHECK(talign.contains("text-align:right"), "table align right");

    // Inline formatting inside table cells
    QString tcell = parser.parse("| Col |\n| --- |\n| **bold** |\n");
    CHECK(tcell.contains("<td><strong>bold</strong></td>"), "table cell inline bold");

    // HTML escaping in code
    CHECK(parser.parse("```\nif (a < b) {}\n```\n").contains("&lt;"), "escape in code");

    if (failures == 0) {
        std::cout << "All Markdown parser tests passed.\n";
        return 0;
    }
    std::cerr << failures << " test(s) failed.\n";
    return 1;
}
