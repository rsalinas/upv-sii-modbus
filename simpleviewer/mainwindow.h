#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void updateStatusBar(const QString &status);

    void on_action_Clear_triggered();

    void on_action_Exit_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
