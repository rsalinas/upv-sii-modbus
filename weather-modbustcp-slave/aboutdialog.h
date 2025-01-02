#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

private:
    QLabel *imageLabel;
    QLabel *textLabel;
    QPushButton *closeButton;
};

#endif // ABOUTDIALOG_H
