#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QObject>
#include <QString>

class MarkdownParser : public QObject
{
    Q_OBJECT
public:
    explicit MarkdownParser(QObject *parent = 0);
    QString parse(const QString &markdown) const;
};

#endif // MARKDOWNPARSER_H
