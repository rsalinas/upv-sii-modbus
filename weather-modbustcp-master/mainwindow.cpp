#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      modbus(new ModbusReader("127.0.0.1", 1502, 1, this)),
      timer(new QTimer(this)),
      chart(new QChart()),
      tempSeries(new QLineSeries()),
      presSeries(new QLineSeries()),
      axisX(new QDateTimeAxis()),
      axisYTemp(new QValueAxis()),
      axisYPres(new QValueAxis())
{
    ui->setupUi(this);

    connect(timer, &QTimer::timeout, this, &MainWindow::updateData);
    connect(ui->checkBoxRed, &QCheckBox::stateChanged, this, &MainWindow::onLedCheckboxChanged);
    connect(ui->checkBoxGreen, &QCheckBox::stateChanged, this, &MainWindow::onLedCheckboxChanged);
    connect(ui->checkBoxBlue, &QCheckBox::stateChanged, this, &MainWindow::onLedCheckboxChanged);

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

    axisYPres->setRange(900, 1100);
    axisYPres->setTitleText("Presión (hPa)");
    chart->addAxis(axisYPres, Qt::AlignRight);
    presSeries->attachAxis(axisYPres);

    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);

    if (modbus->connectDevice()) {
        startTime = QDateTime::currentDateTime();
        timer->start(1000);
    } else {
        ui->statusbar->showMessage("Error al conectar: " + modbus->errorString());
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateData() {
    QVector<quint16> regs = modbus->getInputRegisters(2, 0);
    if (regs.size() < 2) return;

    QDateTime now = QDateTime::currentDateTime();

    tempSeries->append(now.toMSecsSinceEpoch(), regs[1]);
    presSeries->append(now.toMSecsSinceEpoch(), regs[0]);

    axisX->setMin(now.addSecs(-60));
    axisX->setMax(now);
}

void MainWindow::onLedCheckboxChanged()
{
    QBitArray coils(3);
    coils[0] = ui->checkBoxRed->isChecked();
    coils[1] = ui->checkBoxGreen->isChecked();
    coils[2] = ui->checkBoxBlue->isChecked();

    if (!modbus->setCoils(coils)) {
        ui->statusbar->showMessage("Fallo al escribir los LEDs");
    } else {
        ui->statusbar->showMessage("LEDs actualizados");
    }
}
