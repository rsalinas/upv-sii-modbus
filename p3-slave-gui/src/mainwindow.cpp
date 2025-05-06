#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    modbus.coils = {false, false, false};
    discreteInputs = {true, true};
    inputRegisters = {1013, 25};

    connect(ui->sliderPressure,
            &QSlider::valueChanged,
            this,
            &MainWindow::on_sliderPressure_valueChanged);
    connect(ui->sliderTemperature, &QSlider::valueChanged, this, &MainWindow::on_sliderTemperature_valueChanged);
    connect(ui->buttonDiscrete, &QPushButton::pressed, this, &MainWindow::on_buttonDiscrete_pressed);
    connect(ui->buttonDiscrete, &QPushButton::released, this, &MainWindow::on_buttonDiscrete_released);
    connect(ui->buttonDiscrete2, &QPushButton::pressed, this, &MainWindow::on_buttonDiscrete2_pressed);
    connect(ui->buttonDiscrete2, &QPushButton::released, this, &MainWindow::on_buttonDiscrete2_released);

    updateLEDs();
    if (!modbus.listen(QHostAddress::Any, 1502)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        statusBar()->showMessage("Fallo al iniciar el servidor Modbus TCP");
    } else {
        qDebug() << "Servidor Modbus TCP iniciado en el puerto 1502.";
        statusBar()->showMessage("Servidor Modbus TCP iniciado en el puerto 1502");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sliderPressure_valueChanged(int value)
{
    ui->editPressure->setText(QString::number(value));
    inputRegisters[0] = value;
}

void MainWindow::on_sliderTemperature_valueChanged(int value)
{
    ui->editTemperature->setText(QString::number(value));
    inputRegisters[1] = value;
}

void MainWindow::on_buttonDiscrete_pressed()
{
    discreteInputs[0] = true;
}

void MainWindow::on_buttonDiscrete_released()
{
    discreteInputs[0] = false;
}

void MainWindow::on_buttonDiscrete2_pressed()
{
    discreteInputs[1] = true;
}

void MainWindow::on_buttonDiscrete2_released()
{
    discreteInputs[1] = false;
}

void MainWindow::updateLEDs()
{
    ui->ledRed->setStyleSheet(coils[0] ? "background-color: red;" : "background-color: black;");
    ui->ledBlue->setStyleSheet(coils[1] ? "background-color: blue;" : "background-color: black;");
    ui->ledGreen->setStyleSheet(coils[2] ? "background-color: green;" : "background-color: black;");
}
