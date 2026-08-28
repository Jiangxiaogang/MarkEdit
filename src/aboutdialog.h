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
    explicit AboutDialog(QWidget *parent = 0);
};

#endif // ABOUTDIALOG_H
