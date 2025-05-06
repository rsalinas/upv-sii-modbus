#pragma once

#include <QMainWindow>
#include <QtCharts>
#include <QTimer>
#include <QDateTime>
#include "modbusreader.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateData();
    void onLedCheckboxChanged();

private:
    Ui::MainWindow *ui;
    ModbusReader *modbus;
    QTimer *timer;
    QChart *chart;
    QLineSeries *tempSeries;
    QLineSeries *presSeries;
    QDateTimeAxis *axisX;
    QValueAxis *axisYTemp;
    QValueAxis *axisYPres;
    QDateTime startTime;
};