#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QtSerialBus/QModbusTcpServer>
#include "joystickview.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    GraphicsView *joystickView;
    QModbusTcpServer *modbusServer;
    QTreeWidgetItem *joystickWidgetItem;
    QTreeWidgetItem *joystickWidgetItemAxis[2];
    QTreeWidgetItem *discreteInputsWidgetItems[2];
    QTreeWidgetItem *coilsWidgetItems[3];
    QTreeWidgetItem *holdingRegistersWidgetItems[3];
};
#endif // MAINWINDOW_H
