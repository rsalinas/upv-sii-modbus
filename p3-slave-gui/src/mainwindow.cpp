#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    modbus.coils = {false, false, false};
    modbus.discreteInputs = {true, true};
    modbus.inputRegisters = {1013, 25};

    connect(ui->sliderPressure,
            &QSlider::valueChanged,
            this,
            &MainWindow::on_sliderPressure_valueChanged);
    connect(ui->sliderTemperature, &QSlider::valueChanged, this, &MainWindow::on_sliderTemperature_valueChanged);
    connect(ui->buttonDiscrete, &QPushButton::pressed, this, &MainWindow::on_buttonDiscrete_pressed);
    connect(ui->buttonDiscrete, &QPushButton::released, this, &MainWindow::on_buttonDiscrete_released);
    connect(ui->buttonDiscrete2, &QPushButton::pressed, this, &MainWindow::on_buttonDiscrete2_pressed);
    connect(ui->buttonDiscrete2, &QPushButton::released, this, &MainWindow::on_buttonDiscrete2_released);
    connect(&modbus, &ModbusTcpSlave::coilChanged, this, &MainWindow::updateLEDs);

    updateLEDs();
    if (!modbus.listen(QHostAddress::Any, 1502)) {
        qCritical() << "No se pudo iniciar el servidor Modbus TCP.";
        statusBar()->showMessage("Fallo al iniciar el servidor Modbus TCP");
    } else {
        qDebug() << "Servidor Modbus TCP iniciado en el puerto 1502.";
        statusBar()->showMessage("Esclavo Modbus-TCP escuchando en el puerto 1502");
    }
    modbus.discreteInputs[0] = false;
    modbus.discreteInputs[1] = false;
    ui->editPressure->setText(QString::number(ui->sliderPressure->value()));
    ui->editTemperature->setText(QString::number(ui->sliderTemperature->value()));
    connect(ui->editPressure, &QLineEdit::editingFinished, this, [=]() {
        bool ok;
        int val = ui->editPressure->text().toInt(&ok);
        if (ok && val >= ui->sliderPressure->minimum() && val <= ui->sliderPressure->maximum()) {
            ui->sliderPressure->setValue(val);
        }
        if (!ok) {
            QMessageBox::warning(this, "Entrada inválida", "Introduce un número válido.");
        }
    });

    connect(ui->editTemperature, &QLineEdit::editingFinished, this, [=]() {
        bool ok;
        int val = ui->editTemperature->text().toInt(&ok);
        if (ok && val >= ui->sliderTemperature->minimum()
            && val <= ui->sliderTemperature->maximum()) {
            ui->sliderTemperature->setValue(val);
        }
        if (!ok) {
            QMessageBox::warning(this, "Entrada inválida", "Introduce un número válido.");
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_sliderPressure_valueChanged(int value)
{
    ui->editPressure->setText(QString::number(value));
    modbus.inputRegisters[0] = value;
}

void MainWindow::on_sliderTemperature_valueChanged(int value)
{
    ui->editTemperature->setText(QString::number(value));
    modbus.inputRegisters[1] = value;
}

void MainWindow::on_buttonDiscrete_pressed()
{
    modbus.discreteInputs[0] = true;
}

void MainWindow::on_buttonDiscrete_released()
{
    modbus.discreteInputs[0] = false;
}

void MainWindow::on_buttonDiscrete2_pressed()
{
    modbus.discreteInputs[1] = true;
}

void MainWindow::on_buttonDiscrete2_released()
{
    modbus.discreteInputs[1] = false;
}

void MainWindow::updateLEDs()
{
    ui->ledRed->setStyleSheet(modbus.coils[0] ? "background-color: red;"
                                              : "background-color: black;");
    ui->ledBlue->setStyleSheet(modbus.coils[1] ? "background-color: blue;"
                                               : "background-color: black;");
    ui->ledGreen->setStyleSheet(modbus.coils[2] ? "background-color: green;"
                                                : "background-color: black;");
}
