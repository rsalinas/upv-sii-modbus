#include "mainwindow.h"
#include <QDateTime>
#include "modbuscontroller.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      timer(new QTimer(this)),
      chart(new QChart()),
      tempSeries(new QLineSeries()),
      presSeries(new QLineSeries()),
      axisX(new QDateTimeAxis()),
      axisYTemp(new QValueAxis()),
      axisYPres(new QValueAxis())
{
    ui->setupUi(this);
    if (!controller.connectToHost("127.0.0.1", 1502)) {
        qCritical() << "Error: No se pudo conectar al servidor Modbus.";
    }

    connect(timer, &QTimer::timeout, this, &MainWindow::updateData);
    connect(ui->checkBoxRed, &QCheckBox::checkStateChanged, this, &MainWindow::onLedsChanged);

    connect(ui->checkBoxGreen, &QCheckBox::checkStateChanged, this, &MainWindow::onLedsChanged);

    connect(ui->checkBoxBlue, &QCheckBox::checkStateChanged, this, &MainWindow::onLedsChanged);

    chart->legend()->hide();
    chart->addSeries(tempSeries);
    chart->addSeries(presSeries);

    axisX->setFormat("HH:mm:ss");
    axisX->setTitleText("Hora");
    chart->addAxis(axisX, Qt::AlignBottom);
    tempSeries->attachAxis(axisX);
    presSeries->attachAxis(axisX);

    axisYTemp->setRange(0, 100);
    axisYTemp->setTitleText("Temp (ºC)");
    chart->addAxis(axisYTemp, Qt::AlignLeft);
    tempSeries->attachAxis(axisYTemp);

    axisYPres->setRange(950, 1050);
    axisYPres->setTitleText("Presión (hPa)");
    chart->addAxis(axisYPres, Qt::AlignRight);
    presSeries->attachAxis(axisYPres);

    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    timer->start(1000);
    ui->chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onLedsChanged()
{
    controller.setLEDs(
        ui->checkBoxRed->isChecked(),
        ui->checkBoxGreen->isChecked(),
        ui->checkBoxBlue->isChecked()
    );
}

void MainWindow::updateData()
{
    quint16 p, t;
    if (controller.readData(p, t)) {
        QDateTime now = QDateTime::currentDateTime();
        tempSeries->append(now.toMSecsSinceEpoch(), t);
        presSeries->append(now.toMSecsSinceEpoch(), p);
        axisX->setMin(now.addSecs(-60));
        axisX->setMax(now);
        statusBar()->showMessage(QString("Presión: %1 hPa, Temp: %2 ºC").arg(p).arg(t));
    }

    bool btn1, btn2;
    if (controller.readButtons(btn1, btn2)) {
        ui->checkBoxButton1->setChecked(btn1);
        ui->checkBoxButton2->setChecked(btn2);
    }
}
