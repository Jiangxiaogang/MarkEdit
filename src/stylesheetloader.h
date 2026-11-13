#ifndef STYLESHEETLOADER_H
#define STYLESHEETLOADER_H

#include <QString>

// Stateless helper for preview CSS. The built-in style is compiled into the
// binary from styles/default.css as :/styles/default.css.
class StyleSheetLoader
{
public:
    // Returns the built-in preview style. Loaded once and cached.
    static QString defaultStyle();

    // Loads CSS from filePath (UTF-8). An empty path selects the built-in
    // style. On any failure the built-in style is returned and errorMessage
    // (when provided) receives the reason.
    static QString load(const QString &filePath, QString *errorMessage = 0);

    // Checks that braces are balanced. Comments and quoted strings are
    // ignored so braces inside them do not affect the result. On failure,
    // errorMessage (when provided) receives a description with the line.
    static bool validate(const QString &css, QString *errorMessage = 0);

private:
    StyleSheetLoader();
};

#endif // STYLESHEETLOADER_H
