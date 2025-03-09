#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModbusTcpServer>
#include <QTreeWidgetItem>
#include "qpushbutton.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sliderPressure_valueChanged(int value);
    void on_editPressure_editingFinished();
    void on_sliderTemperature_valueChanged(int value);
    void on_editTemperature_editingFinished();
    void on_buttonDiscrete_pressed();
    void on_buttonDiscrete_released();
    void on_buttonDiscrete2_pressed();
    void on_buttonDiscrete2_released();
    void updateLEDs();
    void onCoilsChanged(); 

    void on_action_About_triggered();
    void on_action_Exit_triggered();

private:
    Ui::MainWindow *ui;
    QModbusTcpServer *modbusServer;

    // Punteros para actualizar el árbol de valores Modbus
    QTreeWidgetItem *treeDiscreteInputItem;
    QTreeWidgetItem *treeInputRegistersItem;
    QModbusDataUnit dataUnitInputRegisters;
    QModbusDataUnit dataUnitDiscreteInputs;
    QFrame *ledRed;
    QFrame *ledBlue;
    QFrame *ledGreen;
    QPushButton *buttonDiscrete2;
    QModbusDataUnit dataUnitCoils;
};

#endif // MAINWINDOW_H
