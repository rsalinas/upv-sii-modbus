#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "modbustcp-slave.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_sliderPressure_valueChanged(int value);
    void on_sliderTemperature_valueChanged(int value);
    void on_buttonDiscrete_pressed();
    void on_buttonDiscrete_released();
    void on_buttonDiscrete2_pressed();
    void on_buttonDiscrete2_released();
    void updateLEDs();

private:
    Ui::MainWindow *ui;
    QVector<bool> coils;
    QVector<bool> discreteInputs;
    QVector<quint16> inputRegisters;
    ModbusTcpSlave modbus;
};

#endif // MAINWINDOW_H
