#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

/**
 * @brief "About MarkEdit" information dialog.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);
};

#endif // ABOUTDIALOG_H
